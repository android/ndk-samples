/*
 * Copyright (C) 2010 The Android Open Source Project
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
 *
 */

#include <jni.h>
#include <android/log.h>
#include <android_native_app_glue.h>
#include <cassert>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "native-activity", __VA_ARGS__))

// process input
int32_t handle_input(android_app* app, AInputEvent* event) {
    int32_t event_type = AInputEvent_getType(event);
    switch (event_type) {
        case AINPUT_EVENT_TYPE_MOTION:
            if (AMotionEvent_getAction(event) == AMOTION_EVENT_ACTION_DOWN) {
                ANativeActivity_showSoftInput(app->activity, 0);
            }
            return 1;
        case AINPUT_EVENT_TYPE_KEY:
            LOGI("key pressed: %d", AKeyEvent_getKeyCode(event));
            return 1;
    }
    return 0;
}

void set_content_view(ANativeActivity* activity, int layoutID) {
    JNIEnv *jni;
    activity->vm->AttachCurrentThread(&jni, NULL);

    jclass activityHelper = jni->FindClass("com/example/native_softkeyboard/ActivityHelper");
    assert(activityHelper);
    jmethodID setContentView = jni->GetMethodID(activityHelper, "setContentView", "(Landroid/app/NativeActivity;I)V");
    assert(setContentView);
    jni->CallStaticVoidMethod(activityHelper, setContentView, layoutID);
    activity->vm->DetachCurrentThread();
}

/**
 * Process command.
 */
void handle_cmd(android_app* app, int32_t cmd) {
    switch (cmd) {
        case APP_CMD_INIT_WINDOW:
            // The window is being shown, get it ready.
            if (app->window != NULL) {
                // inflate layout
                set_content_view(app->activity, 0x7f030000);
            }
            break;
        case APP_CMD_TERM_WINDOW:
            break;
    }
}

// entrypoint
void android_main(struct android_app* app) {
    // Make sure glue isn't stripped.
    app_dummy();
    app->onAppCmd = handle_cmd;
    app->onInputEvent = handle_input;


    // loop waiting for stuff to do.

    while (1) {
        // Read all pending events.
        int ident;
        int events;
        struct android_poll_source* source;

        while ((ident=ALooper_pollAll(0, NULL, &events,
                (void**)&source)) >= 0) {

            // Process this event.
            if (source != NULL) {
                source->process(app, source);
            }

            // Check if we are exiting.
            if (app->destroyRequested != 0) {
                return;
            }
        }
    }
}
