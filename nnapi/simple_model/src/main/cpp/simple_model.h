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

/**
 * SimpleModel
 * Build up the hardcoded graph of
 *   Adder1 ---+
 *             +--- Multiplier--->output result
 *   Adder2 ---+
 *
 *   Operands are all 2-D TENSOR_FLOAT32 of:
 *       dimLength x dimLength
 *   with NO fused_activation operation
 *
 */
class SimpleModel {
 public:
  explicit SimpleModel(uint32_t dimLength);
  ~SimpleModel();
  bool IsReady() const;

  /**
   * inputs:
   *   inputVale1: ADDER_ONE 2nd input operand
   *   inputValue2: ADDER_TWO 2nd input operand
   *   result: model's computation result buffer
   *   SimpleModel will duplicate each value as a
   *     dimLength_ x dimLength_
   *   2-D tensor and feed to NNAPI model
   *
   *   Note: 2 adders' 1st input operand is a constant, hardcoded
   *   to kAdder1WeightValue (0.5f) &
   *      kAdder2WeightValue (0.4f)
   *
   */
  bool Compute(float inputValue1, float inputValue2, float* result);

  /*
   * return hardcoded constant value for adder1 and adder2 in the graph
   */
  static float getAdderConstant(uint32_t adderIdx);

 private:
  ANeuralNetworksModel* model_;
  ANeuralNetworksCompilation* compilation_;

  uint32_t dimLength_;
  uint32_t tensorSize_;

  std::vector<float> inputTensor1_;
  std::vector<float> inputTensor2_;
  std::vector<float> outputTensor_;

  std::vector<float> adder1Weights_;
  std::vector<float> adder2Weights_;

  bool modelReady_;

  bool CreateCompiledModel(void);
};

const size_t kUserInputLength = 2;

#endif  // NNAPI_SIMPLE_MODEL_H
