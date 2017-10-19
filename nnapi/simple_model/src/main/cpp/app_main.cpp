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

#include "simple_model.h"
#include <native_debug.h>
#include <jni.h>
#include <string>
#include <cstdlib>
#include <cassert>

class SimpleModelEngine {
 private:
  JavaVM *javaVM_;
  jobject javaMainActivityObj_;

  SimpleModel *simpleModel_;
  volatile bool deletePending_;

 public:
  explicit SimpleModelEngine(JNIEnv *env, jobject obj) {
    jint status = env->GetJavaVM(&javaVM_);
    javaMainActivityObj_ = env->NewGlobalRef(obj);
    assert(status == JNI_OK);

    deletePending_ = false;
    simpleModel_ = new SimpleModel;
  };

  ~SimpleModelEngine() {
    delete simpleModel_;

    JNIEnv *env;
    javaVM_->AttachCurrentThread(&env, NULL);
    env->DeleteGlobalRef(javaMainActivityObj_);
  }

  bool IsReady() { return simpleModel_->IsReady(); }

  bool IsBusy() { return simpleModel_->IsBusy(); }
  void setDeletePending(bool pending) { deletePending_ = pending; }
  bool getDeletePending() { return deletePending_; }

  float Compute(std::vector<float> &inputs) {
    float result = simpleModel_->Compute(inputs);
    float simResult = simpleModel_->SimulatedResult(inputs);
    float delta = result - simResult;
    if (delta < 0.0) {
      delta = -delta;
    }
    assert(delta < FLOAT_EPISILON);
    return result;
  }
  void onCompute(std::vector<float> *userInput);
};

SimpleModelEngine *appEngine = nullptr;

/**
 * Initialize basicModel
 * returns: JNI_TRUE after initialization correctly, JNI_FALSE otherwise
 */
extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_nnapi_simpleModel_MainActivity_basicModeInit(JNIEnv *env,
                                                             jobject instance) {
  if (initNNAPI() < 0) {
    assert(0);
    return JNI_FALSE;
  }
  appEngine = new SimpleModelEngine(env, instance);
  return (appEngine->IsReady() ? JNI_TRUE : JNI_FALSE);
}

/**
 * Perform compute in new thread
 * @param userInput: array of float from user inputs (length = 4 )
 */
void SimpleModelEngine::onCompute(std::vector<float> *userInput) {
  jfloat result = appEngine->Compute(*userInput);
  delete userInput;

  if (appEngine->getDeletePending()) {
    delete appEngine;
    appEngine = nullptr;
  }

  JNIEnv *env;
  javaVM_->AttachCurrentThread(&env, NULL);
  jclass clazz = env->GetObjectClass(javaMainActivityObj_);
  if (!clazz) {
    clazz = env->FindClass("MainActivity");
  }
  jmethodID methodID = env->GetMethodID(clazz, "UpdateResult", "(F)V");

  assert(methodID);
  env->CallVoidMethod(javaMainActivityObj_, methodID, result);
  javaVM_->DetachCurrentThread();
}

/**
 * Compute with input from user for the basic model.
 * inputs: an array of floats with 4 elements for 1st layer adders
 * returns: ouput from MUL ops in the second layer
 */
#include <thread>
extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_nnapi_simpleModel_MainActivity_basicModelCompute(
    JNIEnv *env, jobject instance, jfloatArray inputs) {
  if (!appEngine->IsReady()) {
    LOGE("====SimpleModel is not initialized");
    return JNI_FALSE;
  }

  jfloat *inputData = env->GetFloatArrayElements(inputs, NULL);
  jsize len = env->GetArrayLength(inputs);
  assert(len == kUserInputLength);
  std::vector<float> *modelInputs =
      new std::vector<float>(inputData, &inputData[len]);
  std::thread computeHandler(&SimpleModelEngine::onCompute, appEngine,
                             modelInputs);
  computeHandler.detach();
  env->ReleaseFloatArrayElements(inputs, inputData, 0);
  return JNI_TRUE;
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