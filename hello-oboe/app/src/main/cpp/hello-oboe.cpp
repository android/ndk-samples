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

#include "OboeSinePlayer.h"

static OboeSinePlayer* oboePlayer = nullptr;

/* Create Oboe playback stream
 * Returns:  0 - success
 *          -1 - failed
 */
jint CreateStream(JNIEnv* /* env */, jobject /* this */) {
  oboePlayer = new OboeSinePlayer();

  return oboePlayer ? 0 : -1;
}
void DestroyStream(JNIEnv* /* env */, jobject /* this */) {
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
jint PlaySound(JNIEnv* /* env */, jobject /* this */, jboolean enable) {
  jint result = 0;
  if (oboePlayer) {
    oboePlayer->enable(enable);
  } else {
    result = -1;
  }
  return result;
}

extern "C" JNIEXPORT jint JNI_OnLoad(JavaVM* _Nonnull vm, void* _Nullable) {
  JNIEnv* env;
  if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
    return JNI_ERR;
  }

  jclass c = env->FindClass("com/google/example/hellooboe/MainActivity");
  if (c == nullptr) return JNI_ERR;

  static const JNINativeMethod methods[] = {
      {"createStream", "()I", reinterpret_cast<void*>(CreateStream)},
      {"destroyStream", "()V", reinterpret_cast<void*>(DestroyStream)},
      {"playSound", "(Z)I", reinterpret_cast<void*>(PlaySound)},
  };
  int rc = env->RegisterNatives(c, methods, arraysize(methods));
  if (rc != JNI_OK) return rc;

  return JNI_VERSION_1_6;
}
