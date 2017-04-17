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
#include <cstdio>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "native-activity", __VA_ARGS__))

void toggle_soft_keyboard(android_app* app) {
    JNIEnv *jni;
    app->activity->vm->AttachCurrentThread( &jni, NULL );

    jclass cls = jni->GetObjectClass(app->activity->clazz);
    jmethodID methodID = jni->GetMethodID(cls, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;" );
    jstring service_name = jni->NewStringUTF("input_method");
    jobject input_service = jni->CallObjectMethod(app->activity->clazz, methodID, service_name);

    jclass input_service_cls = jni->GetObjectClass(input_service);
    methodID = jni->GetMethodID(input_service_cls, "toggleSoftInput", "(II)V");
    jni->CallVoidMethod(input_service, methodID, 0, 0);

    jni->DeleteLocalRef(service_name);

    app->activity->vm->DetachCurrentThread();
}

void set_binding(android_app* app, const char* method, const char* value) {
    JNIEnv *jni;
    app->activity->vm->AttachCurrentThread( &jni, NULL );

    jclass cls = jni->GetObjectClass(app->activity->clazz);
    jmethodID getBinding = jni->GetMethodID(cls, "getBinding", "()Lcom/example/native_softkeyboard/databinding/SoftkeyboardActivityBinding;" );
    assert(getBinding);
    jobject binding = jni->CallObjectMethod(app->activity->clazz, getBinding);
    jclass bindingCls = jni->GetObjectClass(binding);
    jmethodID setMessage = jni->GetMethodID(bindingCls, method, "(Ljava/lang/String;)V" );
    assert(setMessage);
    jstring message = jni->NewStringUTF(value);
    jni->CallVoidMethod(binding, setMessage, message);
}

// process input
int32_t handle_input(android_app* app, AInputEvent* event) {
    int32_t event_type = AInputEvent_getType(event);
    switch (event_type) {
        case AINPUT_EVENT_TYPE_MOTION:
            if (AMotionEvent_getAction(event) == AMOTION_EVENT_ACTION_DOWN) {
                toggle_soft_keyboard(app);
            }
            return 1;
        case AINPUT_EVENT_TYPE_KEY:
            int keycode = AKeyEvent_getKeyCode(event);
            LOGI("key pressed: %d", keycode);
            char buf[256];
            sprintf(buf, "keycode: %d", keycode);
            set_binding(app, "setMessage", buf);
            set_binding(app, "setSmallMessage", "some other message");
            return 1;
    }
    return 0;
}

// process command
void handle_cmd(android_app* app, int32_t cmd) {
    switch (cmd) {
        case APP_CMD_INIT_WINDOW:
            // The window is being shown, get it ready.
            if (app->window != NULL) {
                // inflate layout
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
