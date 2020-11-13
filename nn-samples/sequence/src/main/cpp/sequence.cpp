/**
 * Copyright 2020 The Android Open Source Project
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

#include <fcntl.h>
#include <jni.h>
#include <iomanip>
#include <sstream>
#include <string>

#include <android/asset_manager_jni.h>
#include <android/log.h>
#include <android/sharedmem.h>
#include <sys/mman.h>

#include "sequence_model.h"

extern "C" JNIEXPORT jlong JNICALL Java_com_example_android_sequence_MainActivity_initModel(
    JNIEnv* env, jobject /* this */, jfloat ratio) {

  auto model = SimpleSequenceModel::Create(ratio);
  if (model == nullptr) {
    __android_log_print(ANDROID_LOG_ERROR,
                        LOG_TAG,
                        "Failed to create the model.");
    return 0;
  }

  return (jlong) (uintptr_t) model.release();
}

extern "C" JNIEXPORT jfloat JNICALL Java_com_example_android_sequence_MainActivity_compute(
    JNIEnv* env,
    jobject /* this */,
    jfloat initialValue,
    jint steps,
    jlong _nnModel) {
  SimpleSequenceModel* nn_model = (SimpleSequenceModel*) _nnModel;
  float result = 0.0f;
  nn_model->Compute(initialValue, static_cast<uint32_t>(steps), &result);
  return result;
}

extern "C" JNIEXPORT void JNICALL Java_com_example_android_sequence_MainActivity_destroyModel(
    JNIEnv* env, jobject /* this */, jlong _nnModel) {
  SimpleSequenceModel* nn_model = (SimpleSequenceModel*) _nnModel;
  delete (nn_model);
}
