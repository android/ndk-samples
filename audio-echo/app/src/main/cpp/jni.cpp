// Copyright (C) 2025 The Android Open Source Project
// SPDX-License-Identifier: Apache-2.0

#include <base/macros.h>
#include <jni.h>

#include "jni_interface.h"

extern "C" JNIEXPORT jint JNI_OnLoad(JavaVM* _Nonnull vm, void* _Nullable) {
  JNIEnv* env;
  if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
    return JNI_ERR;
  }

  jclass c = env->FindClass("com/google/sample/echo/MainActivity");
  if (c == nullptr) return JNI_ERR;

  static const JNINativeMethod methods[] = {
      {"createSLEngine", "(IIJF)V", reinterpret_cast<void*>(CreateSlEngine)},
      {"deleteSLEngine", "()V", reinterpret_cast<void*>(DeleteSlEngine)},
      {"createSLBufferQueueAudioPlayer", "()Z",
       reinterpret_cast<void*>(CreateSlBufferQueueAudioPlayer)},
      {"deleteSLBufferQueueAudioPlayer", "()V",
       reinterpret_cast<void*>(DeleteSlBufferQueueAudioPlayer)},
      {"createAudioRecorder", "()Z",
       reinterpret_cast<void*>(CreateAudioRecorder)},
      {"deleteAudioRecorder", "()V",
       reinterpret_cast<void*>(DeleteAudioRecorder)},
      {"startPlay", "()V", reinterpret_cast<void*>(StartPlay)},
      {"stopPlay", "()V", reinterpret_cast<void*>(StopPlay)},
      {"configureEcho", "(IF)Z", reinterpret_cast<void*>(ConfigureEcho)},
  };
  int rc = env->RegisterNatives(c, methods, arraysize(methods));
  if (rc != JNI_OK) return rc;

  return JNI_VERSION_1_6;
}
