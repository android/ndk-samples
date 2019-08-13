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
    /* Create Oboe playback stream
     * Returns:  0 - success
     *          -1 - failed
     */
    JNIEXPORT jint JNICALL
    Java_com_google_example_hellooboe_MainActivity_createStream(
            JNIEnv * /* env */,
            jobject /* this */) {
        oboePlayer = new OboeSinePlayer();

        return oboePlayer ? 0 : -1;
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
    /*
     * Play sound with pre-created Oboe stream
     * returns:  0  - success
     *          -1  - failed (stream has not created yet )
     */
    JNIEXPORT jint JNICALL
    Java_com_google_example_hellooboe_MainActivity_playSound(
            JNIEnv * /* env */,
            jobject  /* this */,
            jboolean enable) {
        jint result = 0;
        if (oboePlayer) {
            oboePlayer->enable(enable);
        } else {
            result = -1;
        }
        return result;
    }
}
