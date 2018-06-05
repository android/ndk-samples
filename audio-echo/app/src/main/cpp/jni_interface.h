/*
 * Copyright 2018 The Android Open Source Project
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
#ifndef JNI_INTERFACE_H
#define JNI_INTERFACE_H

#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT void JNICALL Java_com_google_sample_echo_MainActivity_createSLEngine(
    JNIEnv *env, jclass, jint, jint, jlong delayInMs, jfloat decay);
JNIEXPORT void JNICALL Java_com_google_sample_echo_MainActivity_deleteSLEngine(
    JNIEnv *env, jclass type);
JNIEXPORT jboolean JNICALL
Java_com_google_sample_echo_MainActivity_createSLBufferQueueAudioPlayer(
    JNIEnv *env, jclass);
JNIEXPORT void JNICALL
Java_com_google_sample_echo_MainActivity_deleteSLBufferQueueAudioPlayer(
    JNIEnv *env, jclass type);

JNIEXPORT jboolean JNICALL
Java_com_google_sample_echo_MainActivity_createAudioRecorder(JNIEnv *env,
                                                             jclass type);
JNIEXPORT void JNICALL
Java_com_google_sample_echo_MainActivity_deleteAudioRecorder(JNIEnv *env,
                                                             jclass type);
JNIEXPORT void JNICALL
Java_com_google_sample_echo_MainActivity_startPlay(JNIEnv *env, jclass type);
JNIEXPORT void JNICALL
Java_com_google_sample_echo_MainActivity_stopPlay(JNIEnv *env, jclass type);
JNIEXPORT jboolean JNICALL
Java_com_google_sample_echo_MainActivity_configureEcho(JNIEnv *env, jclass type,
                                                       jint delayInMs,
                                                       jfloat decay);
#ifdef __cplusplus
}
#endif

#endif // JNI_INTERFACE_H
