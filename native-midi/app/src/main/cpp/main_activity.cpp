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

extern "C" {

// Data callback stuff
JavaVM* theJvm;
jobject dataCallbackObj;
jmethodID midDataCallback;

/**
 * Initializes JNI interface stuff, specifically the info needed to call back into the Java
 * layer when MIDI data is received.
 */
JNICALL void Java_com_example_nativemidi_MainActivity_initNative(JNIEnv * env, jobject instance) {
    env->GetJavaVM(&theJvm);

    // Setup the receive data callback (into Java)
    jclass clsMainActivity = env->FindClass("com/example/nativemidi/MainActivity");
    dataCallbackObj = env->NewGlobalRef(instance);
    midDataCallback = env->GetMethodID(clsMainActivity, "onNativeMessageReceive", "([B)V");
}

} // extern "C"
