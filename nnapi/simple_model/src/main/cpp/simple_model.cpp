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
#include <string>
#include <cstdlib>
#include <cassert>
#include <map>

/**
 * SimpleModel Constructor
 */
SimpleModel::SimpleModel() {
  model_ = nullptr;
  compilation_ = nullptr;
  execution_ = nullptr;
  modelReady_ = false;
  busy_ = false;

  int32_t status;

  // 1. Create and configure model
  status = ANeuralNetworksModel_create(&model_);
  ANN_ASSERT(status);

  // Create operands (10 in total):
  //   Adder1: 3 (2 for inputs, 1 fuseCode)
  //   Adder2: 3 (2 for inputs, 1 fuseCode)
  //   Mul:    4 (2 for inputs, 1 fuseCode, 1 output)
  //   (2 are shared: adders' outputs and Mul's 2 inputs are the same)
  CreateOperands();

  // Assign input / output operands to our SimpleModel
  std::vector<uint32_t> inputNames;
  std::vector<uint32_t> outputNames;
  for (auto& opr : operands_) {
    // when an operand is input from user, its src_ should be NO_OP
    if (opr.src_ == OP_NAME::NO_OP) {
      opr.inputIdx_ = inputNames.size();
      inputNames.push_back(opr.name_);
      // an edge could be both input AND output for our model.
      assert(opr.outputIdx_ == INVALID_OPERAND_INDEX);
    }
    if (opr.dst_ == OP_NAME::NO_OP) {
      opr.outputIdx_ = outputNames.size();
      outputNames.push_back(opr.name_);
      assert(opr.inputIdx_ == INVALID_OPERAND_INDEX);
    }
  }

  // Identify to model which ones are input/outputs.
  // For scalar type, if the value is
  //   a) not set before calling ANeuralNetworksModel_finish(), it is an input
  //   b) set before calling ANeuralNetworksModel_finish(), it is a constant
  // constant shall NOT be inside inputs for Model_setInputsAndOutputs()
  // Runtime uses these inputs and outputs arrays and index in
  // ANeuralNetworksExecution_setInput(execution_, index, .., data)
  // to connect the data to the right operand:
  //    data --> the operand of inputs[index]
  // same for outputs.
  status = ANeuralNetworksModel_identifyInputsAndOutputs(
      model_, inputNames.size(), inputNames.data(), outputNames.size(),
      outputNames.data());
  ANN_ASSERT(status);

  std::map<OP_NAME, OperationCode> ops = {
      { OP_NAME::ADDER_ONE, ANEURALNETWORKS_ADD },
      { OP_NAME::ADDER_TWO, ANEURALNETWORKS_ADD },
      { OP_NAME::MUL_ONE, ANEURALNETWORKS_MUL },
  };
  for (auto& op : ops) {
    FindOperandsForOp(op.first, inputNames, outputNames);
    status = ANeuralNetworksModel_addOperation(
        model_, op.second, inputNames.size(), inputNames.data(),
        outputNames.size(), outputNames.data());
    ANN_ASSERT(status);
  }

  status = ANeuralNetworksModel_finish(model_);
  ANN_ASSERT(status);

  // 2. Compile the model
  status = ANeuralNetworksCompilation_create(model_, &compilation_);
  ANN_ASSERT(status);

  status = ANeuralNetworksCompilation_finish(compilation_);
  ANN_ASSERT(status);

  // Create an execution (an instance of model), all data goes to
  // execution of the model.
  status = ANeuralNetworksExecution_create(compilation_, &execution_);
  ANN_ASSERT(status);

  // Configure fused_activation functions for all Ops in the model
  // a bit of cautious: setInput() perform lazy copying -- setInput()
  // does a shallow copy first, at compute() stage, the buffer data will
  // be copied over to shared place, and NNAPI runtime works with that copy.
  // So the buffer must be valid after setInput(), until (probably after) calling
  // ANeuralNetworksExecution_compute(). Using static to serve this purpose
  // [ANeuralNetworksExecution_setInputFromMemory() works directly from
  //  the given memory, so no this problem at all and more performance friendly]
  static int32_t fusedActivation = ANEURALNETWORKS_FUSED_NONE;
  {
    std::vector<OP_NAME> ops{OP_NAME::ADDER_ONE, OP_NAME::ADDER_TWO,
                             OP_NAME::MUL_ONE};
    for (auto& op : ops) {
      // fused_activation function is always input index 2 for ADD & MUL
      int32_t index = MapOperationIndexToModelIndex(op, true, 2);
      status = ANeuralNetworksExecution_setInput(execution_, index, nullptr,
                                                 &fusedActivation,
                                                 sizeof(fusedActivation));
      ANN_ASSERT(status);
    }
  }

  // Retrieve model input indices for user inputs: the array sequence must
  // match the data sequence in function Compute(inputs)
  std::vector<std::pair<OP_NAME, uint32_t>> ins{
      std::make_pair(ADDER_ONE, 0), std::make_pair(ADDER_ONE, 1),
      std::make_pair(ADDER_TWO, 0), std::make_pair(ADDER_TWO, 1),
  };
  for (auto in : ins) {
    uint32_t index = MapOperationIndexToModelIndex(in.first, true, in.second);
    assert(index != INVALID_OPERAND_INDEX);
    userInputIndex_.push_back(index);
  }

  modelReady_ = true;
}

/**
 * Creates all needed operands( edges ) for SimpleModel into a structure
 * type struct SimpleOperand; at the same time also assign their usage
 * @return true for success, false otherwise
 */
bool SimpleModel::CreateOperands(void) {
  int32_t status;

  uint32_t dimensions[] = {1};
  ANeuralNetworksOperandType OneDimFloat32Tensor{
      .type = ANEURALNETWORKS_TENSOR_FLOAT32,
      .dimensionCount = 1,
      .dimensions = dimensions,
      .scale = 0.0f,
      .zeroPoint = 0,
  };
  ANeuralNetworksOperandType scalarInt32Type{
      .type = ANEURALNETWORKS_INT32,
      .dimensionCount = 0,
      .dimensions = nullptr,
      .scale = 0.0f,
      .zeroPoint = 0,
  };

  ANeuralNetworksOperandType* operandTypes[] = {
      // Operand1             Operand2              Fused_Activation
      &OneDimFloat32Tensor, &OneDimFloat32Tensor, &scalarInt32Type,
      &OneDimFloat32Tensor, &OneDimFloat32Tensor, &scalarInt32Type,
      &OneDimFloat32Tensor, &OneDimFloat32Tensor, &scalarInt32Type,

      // Output Operands
      &OneDimFloat32Tensor,  // MUL_ONE output
  };

  // Add all operands
  std::vector<uint32_t> operandNames;
  for (uint32_t idx = 0; idx < sizeof(operandTypes) / sizeof(operandTypes[0]);
       idx++) {
    status = ANeuralNetworksModel_addOperand(model_, operandTypes[idx]);
    if (status != ANEURALNETWORKS_NO_ERROR) {
      assert(false);
      return false;
    }

    /*
     * Application must remember the calling sequence to addOperand():
     * the sequence number is the name for the operand. It is the only
     * way to communicate with NNAPI runtime to configure OperationCode;
     * so name is starting with 0 and increase by 1 as operands are added.
     * The return value of addOperand() is error code, NOT operand name.
     */
    operandNames.push_back(idx);
  }

  // Assign operands to model: graph is embedded in operand table.
  operands_.clear();
  SimpleOperand tmpOperand{
      .name_ = operandNames[0],
      .src_ = OP_NAME::NO_OP,
      .srcIdx_ = INVALID_OPERAND_INDEX,
      .dst_ = OP_NAME::ADDER_ONE,
      .dstIdx_ = 0,  // goes to ADDER_ONE's 1st operand
      .inputIdx_ = INVALID_OPERAND_INDEX,
      .outputIdx_ = INVALID_OPERAND_INDEX,
  };
  operands_.push_back(tmpOperand);
  tmpOperand.name_ = operandNames[1];
  tmpOperand.dstIdx_ = 1;
  operands_.push_back(tmpOperand);

  tmpOperand.name_ = operandNames[2];
  tmpOperand.dstIdx_ = 2;  // ADDER_ONE fused_activation
  operands_.push_back(tmpOperand);

  tmpOperand.name_ = operandNames[3];
  tmpOperand.src_ = OP_NAME::NO_OP;
  tmpOperand.srcIdx_ = INVALID_OPERAND_INDEX;
  tmpOperand.dst_ = OP_NAME::ADDER_TWO;  // ADDER_TWO
  tmpOperand.dstIdx_ = 0;
  operands_.push_back(tmpOperand);

  tmpOperand.name_ = operandNames[4];
  tmpOperand.dstIdx_ = 1;
  operands_.push_back(tmpOperand);
  tmpOperand.name_ = operandNames[5];
  tmpOperand.dstIdx_ = 2;
  operands_.push_back(tmpOperand);

  memset(&tmpOperand, 0, sizeof(tmpOperand));
  tmpOperand.name_ = operandNames[6];
  tmpOperand.src_ = OP_NAME::ADDER_ONE;
  tmpOperand.srcIdx_ = 0;  // first and the only output from adder1
  tmpOperand.dst_ = OP_NAME::MUL_ONE;
  tmpOperand.dstIdx_ = 0;
  tmpOperand.inputIdx_ = INVALID_OPERAND_INDEX;
  tmpOperand.outputIdx_ = INVALID_OPERAND_INDEX;
  operands_.push_back(tmpOperand);

  tmpOperand.name_ = operandNames[7];
  tmpOperand.src_ = OP_NAME::ADDER_TWO;
  tmpOperand.srcIdx_ = 0;
  tmpOperand.dstIdx_ = 1;  // second input for MUL op
  operands_.push_back(tmpOperand);

  tmpOperand.name_ = operandNames[8];
  tmpOperand.src_ = OP_NAME::NO_OP;
  tmpOperand.srcIdx_ = INVALID_OPERAND_INDEX;
  tmpOperand.dst_ = OP_NAME::MUL_ONE;
  tmpOperand.dstIdx_ = 2;  // fused_activation function
  tmpOperand.inputIdx_ = INVALID_OPERAND_INDEX;
  tmpOperand.outputIdx_ = INVALID_OPERAND_INDEX;
  operands_.push_back(tmpOperand);

  tmpOperand = {
      .name_ = operandNames[9],
      .src_ = OP_NAME::MUL_ONE,
      .srcIdx_ = 0,
      .dst_ = OP_NAME::NO_OP,
      .dstIdx_ = INVALID_OPERAND_INDEX,
      .inputIdx_ = INVALID_OPERAND_INDEX,
      .outputIdx_ = INVALID_OPERAND_INDEX,
  };
  operands_.push_back(tmpOperand);

  return true;
}

/**
 * Walking through operands_ table to find out all operands for a given OpNode
 * @param name identifies the operation nodes in the simpleMode(adder1, adder1,
 * mul1)
 * @param in to hold the inputs to the OpNode in the right order
 * @param out to hold the outpus to the OpNode in the right order
 * @return true for success, false otherwise
 */
bool SimpleModel::FindOperandsForOp(OP_NAME name, std::vector<uint32_t>& in,
                                    std::vector<uint32_t>& out) {
  std::map<uint32_t, uint32_t> inMap, outMap;
  for (auto& opr : operands_) {
    if (name == opr.src_) {
      outMap[opr.srcIdx_] = opr.name_;
    }
    if (name == opr.dst_) {
      inMap[opr.dstIdx_] = opr.name_;
    }
    assert(opr.src_ != opr.dst_);
  }

  // Operands for any opCode is index from 0 and up,
  // simply move them over from map
  in.resize(0);
  out.resize(0);
  for (int32_t idx = 0; idx < inMap.size(); idx++) {
    in.push_back(inMap[idx]);
  }
  for (int32_t idx = 0; idx < outMap.size(); idx++) {
    out.push_back(outMap[idx]);
  }
  return true;
}

/**
 * Report whether model is constructed and ready to compute
 * @return true if ready to compute, false otherwise
 */
bool SimpleModel::IsReady() { return modelReady_; }

/**
 * SimpleModel is busy computing or not. If it is busy, most likely inside
 * waiting
 * for event.
 * @return true if busy, false otherwise.
 */
bool SimpleModel::IsBusy() { return busy_; }

/**
 * Map opNode's input/output index into SimpleModel's input/output.
 * @param opName is the OpNode's name (ADDER_ONE, ADDER_TWO, MUL_1)
 * @param input mapping for input or output
 * @param opIndex is the index to the opName's local operand Idx
 * @return Model's input/output if the give opIdex is part of SimpleModel's
 *         inputs / outputs; otherwise, INVALID_OPERAND_INDEX
 */
uint32_t SimpleModel::MapOperationIndexToModelIndex(OP_NAME opName, bool input,
                                                    uint32_t opIndex) {
  for (auto& opr : operands_) {
    if (input) {
      // opIndex is input to the opName;
      // from operand (edge) standing point, the opName would be dst_
      if (opr.dst_ == opName && opIndex == opr.dstIdx_) {
        assert(opr.inputIdx_ != INVALID_OPERAND_INDEX);
        return opr.inputIdx_;
      }
    } else {
      // opIndex is output of opName;
      // for operand (edge), the opName would be src_
      if (opr.src_ == opName && opIndex == opr.srcIdx_) {
        assert(opr.outputIdx_ != INVALID_OPERAND_INDEX);
        return opr.outputIdx_;
      }
    }
  }
  // This means caller tries to map an internal operand into input/output
  // which should not happen.
  assert(false);
  return INVALID_OPERAND_INDEX;
}

/**
 * Compute with the given input data
 * @param modelInputs:
 *    1st   Adder1_InputOperand1
 *    2st   Adder1_InputOperand2
 *    3rd   Adder2_InputOperand1
 *    4th   Adder2_InputOperand2
 * @return  computed result, or 0.0f if there is error.
 */
float SimpleModel::Compute(std::vector<float>& modelInputs) {
  int32_t status;

  // 3. compute: configure inputs/output streaming data to the model
  //         All input/output memory locations must be present during compute
  //         period.
  for (int32_t idx = 0; idx < kUserInputLength; idx++) {
    status = ANeuralNetworksExecution_setInput(execution_, userInputIndex_[idx],
                                               nullptr, &modelInputs[idx],
                                               sizeof(modelInputs[idx]));
    ANN_ASSERT(status);
  }

  // The index used in setOutput() is the index to outputs when calling
  // Model_identifyInputsAndOutputs(..., outputSize, outputs); index is not
  // the operand name. Operand name is the application's calling sequence
  // number to Model_AddOperands(); NNAPI does not return, to application,
  // operand name with function Model_AddOperand() call ( the returned value
  // instead is an error code ). It is application's responsibility
  // to keep track of the operand names, and pass the operand names that to
  // be used as inputs/outputs via Model_identifyInputsAndOutputs(); later
  // (like here) application could stream input/output data to input/output.

  // For out simple 2-layer model, we just need to provide a memory location
  // to hold the result.
  float result = 0.0;
  uint32_t index = MapOperationIndexToModelIndex(OP_NAME::MUL_ONE, false, 0);
  status = ANeuralNetworksExecution_setOutput(execution_, index, nullptr,
                                              &result, sizeof(result));
  ANN_ASSERT(status);
  ANeuralNetworksEvent* event;
  status = ANeuralNetworksExecution_startCompute(execution_, &event);
  ANN_ASSERT(status);

  busy_ = true;
  status = ANeuralNetworksEvent_wait(event);
  ANN_ASSERT(status);
  ANeuralNetworksEvent_free(event);
  busy_ = false;

  return result;
}

/**
 * Calculate a simulated version of the model with this apps implementation
 * without using NNAPI.
 *
 * @param inputs inputs to the SimpleModel, refer to function Compute()
 * @return computed result.
 */
float SimpleModel::SimulatedResult(std::vector<float>& inputs) {
  return (inputs[0] + inputs[1]) * (inputs[2] + inputs[3]);
}
/**
 * release NNAPI objects
 * @return none
 */
SimpleModel::~SimpleModel() {
  ANeuralNetworksExecution_free(execution_);
  ANeuralNetworksCompilation_free(compilation_);
  ANeuralNetworksModel_free(model_);
}

