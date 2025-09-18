// Copyright (C) 2025 The Android Open Source Project
// SPDX-License-Identifier: Apache-2.0

#include <base/macros.h>
#include <jni.h>

#include "plasma.h"

extern "C" JNIEXPORT jint JNI_OnLoad(JavaVM* _Nonnull vm, void* _Nullable) {
  JNIEnv* env;
  if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
    return JNI_ERR;
  }

  jclass c = env->FindClass("com/example/plasma/PlasmaView");
  if (c == nullptr) return JNI_ERR;

  static const JNINativeMethod methods[] = {
      {"renderPlasma", "(Landroid/graphics/Bitmap;J)V",
       reinterpret_cast<void*>(RenderPlasma)},
  };
  int rc = env->RegisterNatives(c, methods, arraysize(methods));
  if (rc != JNI_OK) return rc;

  return JNI_VERSION_1_6;
}