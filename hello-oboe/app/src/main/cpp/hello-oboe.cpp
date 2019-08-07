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


static OboeSinePlayer *oboePlayer = nullptr;


extern "C" {
    JNIEXPORT void JNICALL
    Java_com_google_example_hellooboe_MainActivity_createStream(
            JNIEnv * /* env */,
            jobject /* this */) {
        oboePlayer = new OboeSinePlayer();
    }
    JNIEXPORT void JNICALL
    Java_com_google_example_hellooboe_MainActivity_destroyStream(
            JNIEnv * /* env */,
            jobject /* this */) {
        if (oboePlayer) {
            delete oboePlayer;
            oboePlayer = nullptr;
        }
    }
    JNIEXPORT void JNICALL
    Java_com_google_example_hellooboe_MainActivity_enableSound(
            JNIEnv * /* env */,
            jobject  /* this */,
            jboolean enable) {
        if (oboePlayer) {
            oboePlayer->enable(enable);
        }
    }
}