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
#include <base/macros.h>
#include <jni.h>

// Data callback stuff
JavaVM* theJvm;
jobject dataCallbackObj;
jmethodID midDataCallback;

/**
 * Initializes JNI interface stuff, specifically the info needed to call back
 * into the Java layer when MIDI data is received.
 */
void InitNative(JNIEnv* env, jobject instance) {
  env->GetJavaVM(&theJvm);

  // Setup the receive data callback (into Java)
  jclass clsMainActivity =
      env->FindClass("com/example/nativemidi/MainActivity");
  dataCallbackObj = env->NewGlobalRef(instance);
  midDataCallback =
      env->GetMethodID(clsMainActivity, "onNativeMessageReceive", "([B)V");
}

extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* _Nonnull vm,
                                             void* _Nullable) {
  JNIEnv* env;
  if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
    return JNI_ERR;
  }

  jclass c = env->FindClass("com/example/nativemidi/MainActivity");
  if (c == nullptr) return JNI_ERR;

  static const JNINativeMethod methods[] = {
      {"initNative", "()V", reinterpret_cast<void*>(InitNative)},
  };
  int rc = env->RegisterNatives(c, methods, arraysize(methods));
  if (rc != JNI_OK) return rc;

  return JNI_VERSION_1_6;
}
