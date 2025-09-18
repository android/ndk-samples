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

#pragma once

#include <jni.h>

void CreateSlEngine(JNIEnv* env, jclass, jint, jint, jlong delayInMs,
                    jfloat decay);
void DeleteSlEngine(JNIEnv* env, jclass type);
jboolean CreateSlBufferQueueAudioPlayer(JNIEnv* env, jclass);
void DeleteSlBufferQueueAudioPlayer(JNIEnv* env, jclass type);

jboolean CreateAudioRecorder(JNIEnv* env, jclass type);
void DeleteAudioRecorder(JNIEnv* env, jclass type);
void StartPlay(JNIEnv* env, jclass type);
void StopPlay(JNIEnv* env, jclass type);
jboolean ConfigureEcho(JNIEnv* env, jclass type, jint delayInMs, jfloat decay);
