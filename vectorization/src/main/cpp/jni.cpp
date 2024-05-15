/*
 * Copyright (C) 2024 The Android Open Source Project
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

#include <base/logging.h>
#include <jni.h>

#include <expected>
#include <optional>

#include "benchmark.h"

using samples::vectorization::Backend;
using samples::vectorization::BenchmarkMatrixMultiplication;

static jlong BenchmarkMatrixMultiplyJni(JNIEnv* _Nonnull /* env */,
                                        jobject _Nonnull /* this */,
                                        jint backend) {
  auto result = BenchmarkMatrixMultiplication(static_cast<Backend>(backend));
  if (result.has_value()) {
    return result->count();
  }
  return static_cast<jlong>(result.error());
}

JNIEXPORT jint JNI_OnLoad(JavaVM* _Nonnull vm,
                          void* _Nullable reserved __unused) {
  JNIEnv* env;
  if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
    return JNI_ERR;
  }

  jclass c = env->FindClass("com/android/ndk/samples/vectorization/AppJni");
  if (c == nullptr) return JNI_ERR;

  static const JNINativeMethod methods[] = {
      {"benchmarkMatrixMultiply", "(I)J",
       reinterpret_cast<void*>(BenchmarkMatrixMultiplyJni)},
  };
  int rc = env->RegisterNatives(c, methods, arraysize(methods));
  if (rc != JNI_OK) return rc;

  return JNI_VERSION_1_6;
}