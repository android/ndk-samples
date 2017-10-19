/*
 * Copyright (C) 2017 The Android Open Source Project
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

#include <dlfcn.h>
#include <assert.h>

#include "NeuralNetworksApi.h"

int (*ANeuralNetworksMemory_createFromFd)(size_t size, int protect, int fd, size_t offset,
                                          ANeuralNetworksMemory** memory);
void (*ANeuralNetworksMemory_free)(ANeuralNetworksMemory* memory);

int (*ANeuralNetworksModel_create)(ANeuralNetworksModel** model);
void (*ANeuralNetworksModel_free)(ANeuralNetworksModel* model);
int (*ANeuralNetworksModel_finish)(ANeuralNetworksModel* model);
int (*ANeuralNetworksModel_addOperand)(ANeuralNetworksModel* model,
                                       const ANeuralNetworksOperandType* type);
int (*ANeuralNetworksModel_setOperandValue)(ANeuralNetworksModel* model, int32_t index,
                                            const void* buffer, size_t length);
int (*ANeuralNetworksModel_setOperandValueFromMemory)(ANeuralNetworksModel* model, int32_t index,
                                                      const ANeuralNetworksMemory* memory,
                                                      size_t offset, size_t length);
int (*ANeuralNetworksModel_addOperation)(ANeuralNetworksModel* model,
                                         ANeuralNetworksOperationType type, uint32_t inputCount,
                                         const uint32_t* inputs, uint32_t outputCount,
                                         const uint32_t* outputs);
int (*ANeuralNetworksModel_identifyInputsAndOutputs)(ANeuralNetworksModel* model, uint32_t inputCount,
                                                     const uint32_t* inputs, uint32_t outputCount,
                                                     const uint32_t* outputs);
int (*ANeuralNetworksCompilation_create)(ANeuralNetworksModel* model,
                                         ANeuralNetworksCompilation** compilation);
void (*ANeuralNetworksCompilation_free)(ANeuralNetworksCompilation* compilation);
int (*ANeuralNetworksCompilation_setPreference)(ANeuralNetworksCompilation* compilation,
                                                int32_t preference);
int (*ANeuralNetworksCompilation_finish)(ANeuralNetworksCompilation* compilation);
int (*ANeuralNetworksExecution_create)(ANeuralNetworksCompilation* compilation,
                                       ANeuralNetworksExecution** execution);
void (*ANeuralNetworksExecution_free)(ANeuralNetworksExecution* execution);
int (*ANeuralNetworksExecution_setInput)(ANeuralNetworksExecution* execution, int32_t index,
                                         const ANeuralNetworksOperandType* type, const void* buffer,
                                         size_t length);
int (*ANeuralNetworksExecution_setInputFromMemory)(ANeuralNetworksExecution* execution, int32_t index,
                                                   const ANeuralNetworksOperandType* type,
                                                   const ANeuralNetworksMemory* memory, size_t offset,
                                                   size_t length);
int (*ANeuralNetworksExecution_setOutput)(ANeuralNetworksExecution* execution, int32_t index,
                                          const ANeuralNetworksOperandType* type, void* buffer,
                                          size_t length);
int (*ANeuralNetworksExecution_setOutputFromMemory)(ANeuralNetworksExecution* execution, int32_t index,
                                                    const ANeuralNetworksOperandType* type,
                                                    const ANeuralNetworksMemory* memory, size_t offset,
                                                    size_t length);
int (*ANeuralNetworksExecution_startCompute)(ANeuralNetworksExecution* execution,
                                             ANeuralNetworksEvent** event);
int (*ANeuralNetworksEvent_wait)(ANeuralNetworksEvent* event);
void (*ANeuralNetworksEvent_free)(ANeuralNetworksEvent* event);

int32_t initNNAPI(void) {
    static int32_t initCompleted = 0;
    static int32_t initStatus = -1;

    if (initCompleted) {
        return initStatus;
    }
    void *handle = dlopen("libneuralnetworks.so", RTLD_NOW);
    assert(handle);

#   define GET_PROC(s) s=dlsym(handle,#s)
    GET_PROC(ANeuralNetworksMemory_createFromFd);
    GET_PROC(ANeuralNetworksMemory_free);
    GET_PROC(ANeuralNetworksModel_create);
    GET_PROC(ANeuralNetworksModel_free);
    GET_PROC(ANeuralNetworksModel_finish);
    GET_PROC(ANeuralNetworksModel_addOperand);
    GET_PROC(ANeuralNetworksModel_setOperandValue);
    GET_PROC(ANeuralNetworksModel_setOperandValueFromMemory);
    GET_PROC(ANeuralNetworksModel_addOperation);
    GET_PROC(ANeuralNetworksModel_identifyInputsAndOutputs);
    GET_PROC(ANeuralNetworksCompilation_create);
    GET_PROC(ANeuralNetworksCompilation_free);
    GET_PROC(ANeuralNetworksCompilation_setPreference);
    GET_PROC(ANeuralNetworksCompilation_finish);
    GET_PROC(ANeuralNetworksExecution_create);
    GET_PROC(ANeuralNetworksExecution_free);
    GET_PROC(ANeuralNetworksExecution_setInput);
    GET_PROC(ANeuralNetworksExecution_setInputFromMemory);
    GET_PROC(ANeuralNetworksExecution_setOutput);
    GET_PROC(ANeuralNetworksExecution_setOutputFromMemory);
    GET_PROC(ANeuralNetworksExecution_startCompute);
    GET_PROC(ANeuralNetworksEvent_wait);
    GET_PROC(ANeuralNetworksEvent_free);

#   undef GET_PROC

    initCompleted = 1;
    if (ANeuralNetworksMemory_createFromFd &&
        ANeuralNetworksMemory_free  &&
        ANeuralNetworksModel_create &&
        ANeuralNetworksModel_free &&
        ANeuralNetworksMemory_free &&
        ANeuralNetworksModel_finish &&
        ANeuralNetworksModel_addOperand &&
        ANeuralNetworksModel_setOperandValue &&
        ANeuralNetworksModel_setOperandValueFromMemory &&
        ANeuralNetworksModel_addOperation &&
        ANeuralNetworksModel_identifyInputsAndOutputs &&
        ANeuralNetworksCompilation_create &&
        ANeuralNetworksCompilation_free &&
        ANeuralNetworksCompilation_setPreference &&
        ANeuralNetworksCompilation_finish &&
        ANeuralNetworksExecution_create &&
        ANeuralNetworksExecution_free &&
        ANeuralNetworksExecution_setInput &&
        ANeuralNetworksExecution_setInputFromMemory &&
        ANeuralNetworksExecution_setOutput &&
        ANeuralNetworksExecution_setOutputFromMemory &&
        ANeuralNetworksExecution_startCompute &&
        ANeuralNetworksEvent_wait &&
        ANeuralNetworksEvent_free) {
        initStatus = 0;
    }

    return initStatus;
}
