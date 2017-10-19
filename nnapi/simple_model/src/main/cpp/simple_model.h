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

#ifndef NNAPI_SIMPLE_MODEL_H_H
#define NNAPI_SIMPLE_MODEL_H_H

#include <wrapper/NeuralNetworksApi.h>
#include <vector>

#define FLOAT_EPISILON (1e-6)

enum OP_NAME {
  NO_OP,
  ADDER_ONE,
  ADDER_TWO,
  MUL_ONE,
  OP_COUNT,
};

#define INVALID_OPERAND_INDEX (0xFFFFFFFF)
struct SimpleOperand {
  uint32_t name_;  // Operand name is the index when it is adding to NNAPI model
                   // application must keep the track of calling sequence to
                   // ANeuralNetworksModel_addOperand()
  OP_NAME src_;    // OpNode that outputs to this operand (could be NO_OP
                   // when it is
                   // user input
  uint32_t srcIdx_;  // output index of producing OpNode
  OP_NAME dst_;  // OpNode that receiving this operand (could be NO_OP when
                 // this operand is part of model outputs
  uint32_t dstIdx_;  // Input parameter idx of receiving OpNode for this operand

  uint32_t inputIdx_;   // location inside inputs when calling
                        // identifyInputsAndOutputs()
  uint32_t outputIdx_;  // location inside outputs when calling
                        // identifyInputsAndOutputs()
};

/**
 * SimpleModel
 * Build up the hardcoded graph of
 *   Adder1 ---+
 *             +--- Multiplier--->output result
 *   Adder2 ---+
 *
 *   Operands are all 1-D TENSOR_FLOAT32, with NO fused_activation
 */
class SimpleModel {
 public:
  explicit SimpleModel();
  ~SimpleModel();
  bool IsReady();
  bool IsBusy();
  /**
   * inputs( array of float32):
   *   index 0: ADDER_ONE 1st input operand
   *   index 1: ADDER_ONE 2nd input operand
   *   index 2: ADDER_TWO 1st input operand
   *   index 3: ADDER_TWO 2nd input operand
   */
  float Compute(std::vector<float>& inputs);

  /**
   * calculate the result without using NNAPI
   * for result comparison purpose
   */
  float SimulatedResult(std::vector<float>& inputs);

 private:
  ANeuralNetworksModel* model_;
  ANeuralNetworksCompilation* compilation_;
  ANeuralNetworksExecution* execution_;
  bool modelReady_;
  volatile bool busy_;
  std::vector<SimpleOperand> operands_;
  std::vector<uint32_t> userInputIndex_;

  bool CreateOperands(void);
  bool FindOperandsForOp(OP_NAME name, std::vector<uint32_t>& in,
                         std::vector<uint32_t>& out);
  uint32_t MapOperationIndexToModelIndex(OP_NAME opName, bool input,
                                         uint32_t opIndex);
};

#define ANN_ASSERT(v) assert((v) == ANEURALNETWORKS_NO_ERROR)

const size_t kUserInputLength = 4;

#endif  // NNAPI_SIMPLE_MODEL_H_H
