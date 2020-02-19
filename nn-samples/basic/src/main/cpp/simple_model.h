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

#ifndef NNAPI_SIMPLE_MODEL_H
#define NNAPI_SIMPLE_MODEL_H

#include <android/NeuralNetworks.h>
#include <vector>

#define FLOAT_EPISILON (1e-6)
#define TENSOR_SIZE 200
#define LOG_TAG "NNAPI_BASIC"

/**
 * SimpleModel
 * Build up the hardcoded graph of
 *   ADD_1 ---+
 *            +--- MUL--->output result
 *   ADD_2 ---+
 *
 *   Operands are all 2-D TENSOR_FLOAT32 of:
 *       dimLength x dimLength
 *   with NO fused_activation operation
 *
 */
class SimpleModel {
public:
    explicit SimpleModel(size_t size, int protect, int fd, size_t offset);
    ~SimpleModel();

    bool CreateCompiledModel();
    bool Compute(float inputValue1, float inputValue2, float *result);

private:
    ANeuralNetworksModel *model_;
    ANeuralNetworksCompilation *compilation_;
    ANeuralNetworksMemory *memoryModel_;
    ANeuralNetworksMemory *memoryInput2_;
    ANeuralNetworksMemory *memoryOutput_;

    uint32_t dimLength_;
    uint32_t tensorSize_;
    size_t offset_;

    std::vector<float> inputTensor1_;
    int modelDataFd_;
    int inputTensor2Fd_;
    int outputTensorFd_;
};

#endif  // NNAPI_SIMPLE_MODEL_H