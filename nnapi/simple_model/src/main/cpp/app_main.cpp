/**
 * Copyright 2017 The Android Open Source Project
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
#include "application_engine.h"

// Global object for application engine
ApplicationEngine *appEngine = nullptr;

/**
 * Initialize basicModel
 * returns: JNI_TRUE after initialized correctly, JNI_FALSE otherwise
 */
extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_nnapi_simpleModel_MainActivity_basicModelInit(JNIEnv *env,
                                                             jobject instance) {
  JavaVM *javaVM;
  jint status = env->GetJavaVM(&javaVM);
  jobject javaMainActivityObj = env->NewGlobalRef(instance);
  if (status != JNI_OK || javaMainActivityObj == nullptr) {
    return JNI_FALSE;
  }

  appEngine = new ApplicationEngine(javaVM, javaMainActivityObj);
  bool initSuccess = false;
  if (appEngine && appEngine->IsReady()) {
    initSuccess = true;
  }
  return initSuccess;
}

/**
 * return native side hardcoded constant value for
 * ADDER1_INPUT1 in the graph
 */
extern "C" JNIEXPORT jfloat JNICALL
Java_com_google_nnapi_simpleModel_MainActivity_basicModelGetAdder1ConstantValue(
    JNIEnv *env, jobject instance) {

  if (appEngine->IsReady()) {
    return appEngine->getAdderConstant(0);
  } else {
    return 0.0f;
  }
}

extern "C" JNIEXPORT jfloat JNICALL
Java_com_google_nnapi_simpleModel_MainActivity_basicModelGetAdder2ConstantValue(
    JNIEnv *env, jobject instance) {

  if (appEngine->IsReady()) {
    return appEngine->getAdderConstant(1);
  } else {
    return 0.0f;
  }
}

/**
 * Compute with input from user for the simple model.
 * inputs: an array of floats with 2 elements for 1st layer adders
 * returns: ouput from MUL ops in the second layer
 */
extern "C" JNIEXPORT jfloat JNICALL
Java_com_google_nnapi_simpleModel_MainActivity_basicModelCompute(
    JNIEnv *env, jobject instance, jfloatArray inputs) {
  if (!appEngine->IsReady()) {
    LOGE("====SimpleModel is not initialized");
    return 0.0f;
  }

  jfloat *inputData = env->GetFloatArrayElements(inputs, NULL);
  jsize len = env->GetArrayLength(inputs);
  assert(len == kUserInputLength);

  jfloat result = appEngine->Compute(inputData[0], inputData[1]);

  env->ReleaseFloatArrayElements(inputs, inputData, 0);

  return result;
}

/**
 *  cleanup basicModel
 */
extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_nnapi_simpleModel_MainActivity_basicModelFinish(
    JNIEnv *env, jobject instance) {
  if (!appEngine) return JNI_TRUE;

  if (appEngine->IsBusy()) {
    appEngine->setDeletePending(true);
    return JNI_TRUE;
  }
  delete appEngine;
  appEngine = nullptr;
  return JNI_TRUE;
}