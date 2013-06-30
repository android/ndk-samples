/*
 * Copyright 2013 The Android Open Source Project
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

#pragma once

#include <jni.h>
#include <errno.h>
#include <vector>
#include <map>
#include <fstream>
#include <iostream>

#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include <android/sensor.h>
#include <android/log.h>
#include <android_native_app_glue.h>


#define APP_NAME "native-activity"
#define CLASS_NAME "android/app/NativeActivity"
#define APPLICATION_CLASS_NAME "com/sample/mmdPlay/NDKSupport"


#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, APP_NAME, __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, APP_NAME, __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, APP_NAME, __VA_ARGS__))

jclass retrieveClass(JNIEnv *jni, ANativeActivity* activity, const char* className);

class JNIHelper
{
    static ANativeActivity* _activity;
    static jobject _objJNIHelper;
    static jclass _clsJNIHelper;

    static jstring getExternalFilesDir( JNIEnv *env );
public:
    JNIHelper() {
    };
    ~JNIHelper() {
        JNIEnv *env;
        _activity->vm->AttachCurrentThread(&env, NULL);

        env->DeleteGlobalRef(_objJNIHelper);
        env->DeleteGlobalRef(_clsJNIHelper);

        _activity->vm->DetachCurrentThread();

    };
    static void init( ANativeActivity* activity )
    {
        _activity = activity;

    };

    static bool readFile( const char* fileName, std::vector<uint8_t>& buffer );
    static uint32_t loadTexture(const char* fileName );


};
