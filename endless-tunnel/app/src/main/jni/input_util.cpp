/*
 * Copyright (C) Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <dlfcn.h>
#include "input_util.hpp"
#include "joystick-support.hpp"
#include "our_key_codes.hpp"
#include "util.hpp"

// cached device motion ranges
struct DeviceMotionRange {
    int deviceId;
    int source;
    float minX, maxX, minY, maxY;
};
#define MOTION_RANGE_CACHE_MAX 128
struct DeviceMotionRange _motion_range_cache[MOTION_RANGE_CACHE_MAX];
int _motion_range_cache_items = 0;

static bool _init_done = false;
static bool _key_state[OURKEY_COUNT] = {0};
static _getAxisValue_sig _getAxisValue = NULL;

static void _init() {
    if (_init_done) {
        return;
    }

    _init_done = true;

    // look up the AMotionEvent_getAxisValue function
    void *lib_android;
    LOGD("Trying to dlopen libandroid.so");
    if ( (lib_android = dlopen("libandroid.so", 0)) ) {
        LOGD("Opened libandroid.so, looking for AMotionEvent_getAxisValue.");
        _getAxisValue = (_getAxisValue_sig) dlsym(lib_android, "AMotionEvent_getAxisValue");
        LOGD("AMotionEvent_getAxisValue() address is %p", _getAxisValue);
    } else {
        LOGD("Failed to open libandroid.so.");
    }
}

static int _translate_keycode(int code) {
    switch (code) {
        case AKEYCODE_DPAD_LEFT:
            return OURKEY_LEFT;
        case AKEYCODE_DPAD_RIGHT:
            return OURKEY_RIGHT;
        case AKEYCODE_DPAD_UP:
            return OURKEY_UP;
        case AKEYCODE_DPAD_DOWN:
            return OURKEY_DOWN;
        case AKEYCODE_DPAD_CENTER:
        case AKEYCODE_ENTER:
        case AKEYCODE_BUTTON_X:
        case AKEYCODE_BUTTON_A:
            return OURKEY_ENTER;
        case AKEYCODE_BUTTON_Y:
        case AKEYCODE_BUTTON_B:
            return OURKEY_ESCAPE;
        default:
            return -1;
    }
}

static void _report_key_state(int keyCode, bool state, CookedEventCallback callback) {
    bool wentDown = !_key_state[keyCode] && state;
    bool wentUp = _key_state[keyCode] && !state;
    _key_state[keyCode] = state;

    struct CookedEvent ev;
    memset(&ev, 0, sizeof(struct CookedEvent));
    ev.keyCode = keyCode;

    if (wentUp) {
        ev.type = COOKED_EVENT_TYPE_KEY_UP;
        callback(&ev);
    } else if (wentDown) {
        ev.type = COOKED_EVENT_TYPE_KEY_DOWN;
        callback(&ev);
    }
}

static void _report_key_states_from_axes(float x, float y, CookedEventCallback callback) {
    _report_key_state(OURKEY_LEFT, x < -0.5f, callback);
    _report_key_state(OURKEY_RIGHT, x > 0.5f, callback);
    _report_key_state(OURKEY_UP, y < -0.5f, callback);
    _report_key_state(OURKEY_DOWN, y > 0.5f, callback);
}

static bool _process_keys(bool isJoy, AInputEvent *event, CookedEventCallback callback) {
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_KEY) {
        int action = AKeyEvent_getAction(event);
        int code = _translate_keycode(AKeyEvent_getKeyCode(event));
        bool handled = code >= 0;
        if (code >= 0 && action == AKEY_EVENT_ACTION_DOWN) {
            _report_key_state(code, true, callback);
        } else if (code >= 0 && action == AKEY_EVENT_ACTION_UP) {
            _report_key_state(code, false, callback);
        }
        return handled;
    } else if (isJoy) {
        // use joystick axes to emulate directional key events (we could leave this
        // up to the platform, but joystick-to-dpad conversion doesn't work
        // on NDK on older devices,  so we implement manually for maximum compatibility)
        float x = AMotionEvent_getX(event, 0);
        float y = AMotionEvent_getY(event, 0);
        if (_getAxisValue) {
            // take the hat switches into account too, so that either the
            // regular axes or the hat axes can be used to navigate UIs
            x += _getAxisValue(event, AXIS_HAT_X, 0);
            y += _getAxisValue(event, AXIS_HAT_Y, 0);
            x = Clamp(x, -1.0f, 1.0f);
            y = Clamp(y, -1.0f, 1.0f);
        }
        _report_key_states_from_axes(x, y, callback);
        return true;
    }
    return false;
}

static void _look_up_motion_range(int deviceId, int source,
        float *outMinX, float *outMaxX, float *outMinY, float *outMaxY) {
    int i;
    for (i = 0; i < _motion_range_cache_items; i++) {
        DeviceMotionRange *item = &_motion_range_cache[i];
        if (item->deviceId == deviceId && item->source == source) {
            *outMinX = item->minX;
            *outMaxX = item->maxX;
            *outMinY = item->minY;
            *outMaxY = item->maxY;
            return;
        }
    }

    DeviceMotionRange *newItem;
    if (_motion_range_cache_items >= MOTION_RANGE_CACHE_MAX) {
        static bool warned = false;
        if (!warned) {
            LOGW("**** Warning: Motion range cache exceeded. This shouldn't normally happen.");
            warned = true;
        }
        // as an emergency measure, overwrite (arbitrarily) the 1st entry:
        newItem = &_motion_range_cache[i];
    } else {
        // create a new entry
        newItem = &_motion_range_cache[_motion_range_cache_items++];
    }

    LOGD("New device/source pair %d,%d. Querying motion range via JNI.", deviceId, source);

    newItem->deviceId = deviceId;
    newItem->source = source;

    LOGD("====Calling _look_up_motion_range() for device %d", deviceId);
#if 0
    /*
     * What this is ?
     */
    BGNActivity_GetDeviceMotionRange(deviceId, source, &(newItem->minX), &(newItem->maxX),
            &(newItem->minY), &(newItem->maxY));
#endif
    LOGD("Motion range for (device %d, source %d) is X:%f-%f, Y:%f-%f",
            deviceId, source, newItem->minX, newItem->maxX, newItem->minY, newItem->maxY);
    *outMinX = newItem->minX;
    *outMaxX = newItem->maxX;
    *outMinY = newItem->minY;
    *outMaxY = newItem->maxY;
}


static bool CookEvent_Joy(AInputEvent *event, CookedEventCallback callback) {
    struct CookedEvent ev;
    memset(&ev, 0, sizeof(ev));
    ev.type = COOKED_EVENT_TYPE_JOY;
    ev.joyX = AMotionEvent_getX(event, 0);
    ev.joyY = AMotionEvent_getY(event, 0);
    _process_keys(true, event, callback);
    return callback(&ev);
}

static bool CookEvent_Motion(AInputEvent *event, CookedEventCallback callback) {
    int src = AInputEvent_getSource(event);
    int action = AMotionEvent_getAction(event);
    int actionMasked = action & AMOTION_EVENT_ACTION_MASK;
    int ptrIndex = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >>
            AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;

    struct CookedEvent ev;
    memset(&ev, 0, sizeof(ev));

    if (actionMasked == AMOTION_EVENT_ACTION_DOWN || actionMasked ==
            AMOTION_EVENT_ACTION_POINTER_DOWN) {
        ev.type = COOKED_EVENT_TYPE_POINTER_DOWN;
    } else if (actionMasked == AMOTION_EVENT_ACTION_UP || actionMasked ==
            AMOTION_EVENT_ACTION_POINTER_UP) {
        ev.type = COOKED_EVENT_TYPE_POINTER_UP;
    } else {
        ev.type = COOKED_EVENT_TYPE_POINTER_MOVE;
    }

    ev.motionPointerId = AMotionEvent_getPointerId(event, ptrIndex);
    ev.motionIsOnScreen = (src == AINPUT_SOURCE_TOUCHSCREEN);
    ev.motionX = AMotionEvent_getX(event, ptrIndex);
    ev.motionY = AMotionEvent_getY(event, ptrIndex);

    if (ev.motionIsOnScreen) {
        // use screen size as the motion range
        ev.motionMinX = 0.0f;
        ev.motionMaxX = SceneManager::GetInstance()->GetScreenWidth();
        ev.motionMinY = 0.0f;
        ev.motionMaxY = SceneManager::GetInstance()->GetScreenHeight();
    } else {
        // look up motion range for this device
        _look_up_motion_range((int) AInputEvent_getDeviceId(event),
            (int)AInputEvent_getSource(event), &ev.motionMinX, &ev.motionMaxX,
            &ev.motionMinY, &ev.motionMaxY);
    }

    // deliver event
    callback(&ev);

    // deliver motion info about other pointers (for multi-touch)
    int ptrCount = AMotionEvent_getPointerCount(event);
    for (int i = 0; i < ptrCount; i++) {
        ev.type = COOKED_EVENT_TYPE_POINTER_MOVE;
        ev.motionX = AMotionEvent_getX(event, i);
        ev.motionY = AMotionEvent_getY(event, i);
        ev.motionPointerId = AMotionEvent_getPointerId(event, i);
        callback(&ev);        
    }

    // If this is a touch-nav event, return false to indicate that we haven't handled it.
    // This will trigger translation of swipes to DPAD keys, which is what we want.
    // Otherwise, we say that we've handled it.
    return (src != SOURCE_TOUCH_NAVIGATION);
}

bool CookEvent(AInputEvent *event, CookedEventCallback callback) {
    int type = AInputEvent_getType(event);
    int src = AInputEvent_getSource(event);
    bool isJoy = (type == AINPUT_EVENT_TYPE_MOTION) && (src & AINPUT_SOURCE_CLASS_MASK) ==
            SOURCE_CLASS_JOYSTICK;

    if (!_init_done) {
        _init();
        _init_done = true;
    }

    if (isJoy) {
        return CookEvent_Joy(event, callback);
    } else if (type == AINPUT_EVENT_TYPE_KEY) {
        bool handled = _process_keys(false, event, callback);
        if (AKeyEvent_getKeyCode(event) == AKEYCODE_BACK && 0 == AKeyEvent_getAction(event)) {
            // back key was pressed
            struct CookedEvent ev;
            memset(&ev, 0, sizeof(ev));
            ev.type = COOKED_EVENT_TYPE_BACK;
            return callback(&ev);
        }
        // Note: if you want to handle other keys, add code here. For now we only
        // handle DPAD keys as indicated in _process_keys.
        return handled;
    } else if (type == AINPUT_EVENT_TYPE_MOTION) {
        return CookEvent_Motion(event, callback);
    }

    return false;
}

