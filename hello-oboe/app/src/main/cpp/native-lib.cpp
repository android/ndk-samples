/*
 * Copyright (C) 2019 The Android Open Source Project
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
#include "OboeSinePlayer.h"


OboeSinePlayer *ptr;


extern "C" {
    JNIEXPORT void JNICALL
    Java_com_google_example_hellooboe_MainActivity_createStream(
            JNIEnv * /* env */,
            jobject /* this */) {
        ptr = new OboeSinePlayer();
    }
    JNIEXPORT void JNICALL
    Java_com_google_example_hellooboe_MainActivity_destroyStream(
            JNIEnv * /* env */,
            jobject /* this */) {
        delete ptr;
    }
    JNIEXPORT void JNICALL
    Java_com_google_example_hellooboe_MainActivity_enableStream(
            JNIEnv * /* env */,
            jobject  /* this */,
            jboolean enable) {
        ptr->enable(enable);
    }
}