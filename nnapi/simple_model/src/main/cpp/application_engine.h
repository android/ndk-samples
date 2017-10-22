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
#ifndef SAMPLE_APPLICATION_ENGINE_H
#define SAMPLE_APPLICATION_ENGINE_H

#include "simple_model.h"
#include <native_debug.h>
#include <jni.h>
#include <string>
#include <cstdlib>
#include <cassert>
#include <thread>

const uint32_t kArraySize = 200;

/*
 * Application Engine, tracking application side states:
 *    Java UI interaction
 *    simpleModel object
 * It is a small stub class to just keep app up. Main code
 * is in the NNAPI model's creation and computation.
 */
class ApplicationEngine {
private:
    // cache javaVM_ and java side mainActivity object so we could
    // update java UI when computation is completed in JNI side
    JavaVM *javaVM_;
    jobject javaMainActivityObj_;

    SimpleModel *simpleModel_;
    bool isBusy_;

    // Application state: need it in case Java side asks us to terminate
    // but we are still computing in another thread. When this
    // happens, app defers the "delete" request and performs "delete" action
    // after computation is completed.
    volatile bool deletePending_;

public:
    explicit ApplicationEngine(JavaVM *javaVM, jobject activityObj) :
            javaVM_(javaVM), javaMainActivityObj_(activityObj ),
            deletePending_(false), isBusy_(false) {
        simpleModel_ = new SimpleModel(kArraySize);
    }

    ~ApplicationEngine() {
        delete simpleModel_;

        JNIEnv *env;
        javaVM_->AttachCurrentThread(&env, NULL);
        env->DeleteGlobalRef(javaMainActivityObj_);
        javaVM_->DetachCurrentThread();
    }

    bool IsReady() const {
        return simpleModel_->IsReady();
    }

    bool IsBusy() const {
        return isBusy_;
    }
    void setDeletePending(bool pending) {
        deletePending_ = pending;
    }

    float getAdderConstant(int32_t adderIndex) const {
        return simpleModel_->getAdderConstant(adderIndex);
    }
    float Compute(float val1, float val2) {
        isBusy_ = true;
        float result = 0.0f;
        if (!simpleModel_->Compute(val1, val2, &result)) {
            LOGE("Failed from compute");
        }
        isBusy_ = false;

        return result;
    }
};

#endif // SAMPLE_APPLICATION_ENGINE_H
