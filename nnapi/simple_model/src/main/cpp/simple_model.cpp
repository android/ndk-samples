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
#include "native_debug.h"
#include <string>

// a macro to save some typing, note that v must be an error code
// could not be a function call.
#define RETURN_FALSE_ON_FAILURE(v)                             \
  do {                                                                       \
    if (v != ANEURALNETWORKS_NO_ERROR) {                                     \
      LOGE("ANN failed with error code (%d), in %s at line %d", v, __FILE__, \
           __LINE__);                                                        \
      return false;                                                          \
    }                                                                        \
  } while (0)

// Hardcoded constant value for adder1Input1
const float kAdder1WeightValue = .5f;
const float kAdder2WeightValue = .4f;

/**
 * SimpleModel Constructor
 */
SimpleModel::SimpleModel(uint32_t dimLength) :
    model_(nullptr),
    compilation_(nullptr),
    modelReady_ (false),
    dimLength_(dimLength)
{

  tensorSize_ = dimLength_ * dimLength_;

  if (!CreateCompiledModel()) {
    return;
  }

  // reserve memory for 2 input tensors and 1 output tensor
  inputTensor1_.resize(tensorSize_);
  inputTensor2_.resize(tensorSize_);
  outputTensor_.resize(tensorSize_);

  modelReady_ = true;
}

/**
 * We create a graph that consists of three operations: two additions and a
 * multiplication.
 * The sums created by the additions are the inputs to the multiplication. In
 * essence, we are creating a graph that computes (a + b)// (c + d).
 *
 * Two of the four tensors being added will be constants, defined in the
 * model. They represent the weights that would have been learned during a training
 * process. The other two tensors will be inputs to the model. Their values will be
 * provided when we execute the model. They can change from execution to execution.
 *
 * Besides the two input tensors, an optional fused activation function can
 * also be defined for additions and multiplications. In this example,
 * we'll not use an activation function.
 *
 * The graph then has 10 operands:
 *  - 2 tensors that are inputs to the model. These are fed to the two
 *      additions.
 *  - 2 constant tensors that are the other two inputs to the additions.
 *  - 3 fuse activation codes for the two additions and the multiplication.
 *  - 2 intermediate tensors, outputs of the additions and inputs to the
 *      multiplication.
 *  - 1 model output.
 * @return true for success, false otherwise
 */
bool SimpleModel::CreateCompiledModel(void) {
  int32_t status;

  adder1Weights_.resize(tensorSize_);
  adder2Weights_.resize(tensorSize_);

  // Create the model
  status = ANeuralNetworksModel_create(&model_);
  RETURN_FALSE_ON_FAILURE(status);

  uint32_t dimensions[] = {dimLength_, dimLength_};
  ANeuralNetworksOperandType squareFloat32TensorType {
      .type = ANEURALNETWORKS_TENSOR_FLOAT32,
      .dimensionCount = sizeof(dimensions) / sizeof(dimensions[0]),
      .dimensions = dimensions,
      .scale = 0.0f,
      .zeroPoint = 0,
  };
  ANeuralNetworksOperandType scalarInt32Type {
      .type = ANEURALNETWORKS_INT32,
      .dimensionCount = 0,
      .dimensions = nullptr,
      .scale = 0.0f,
      .zeroPoint = 0,
  };

  // Add all operands & operations
  // Operands are implicitly identified by the order in which they are added to the model.
  // The first operand added will have the index value of 0. The second operand will have
  // an index value of 1, the third a value of 2, and so on.
  //
  // These index values are used when specifying the operands used by an operation,
  // when specifying the value of constant operands, and when specifying the inputs
  // and outputs of the model.
  //
  // These indexes are not returned by the model_addOperand call. The application must
  // manage these values. We do this with opIdx.
  uint32_t opIdx = 0;

  // Add operands used by the first adder
  status = ANeuralNetworksModel_addOperand(model_, &squareFloat32TensorType);
  RETURN_FALSE_ON_FAILURE(status);
  uint32_t adder1Input1OperandId_ = opIdx++;

  // The first tensor to be added is a constant tensor that was established
  // during training. We simulate this training here by setting all elements
  // of the tensor to kAdder1WeightValue.
  // For a real program, we would likely read these values from a file.
  std::fill(adder1Weights_.data(),
            adder1Weights_.data() + dimLength_ * dimLength_,
            kAdder1WeightValue);
  status = ANeuralNetworksModel_setOperandValue(
      model_, adder1Input1OperandId_, adder1Weights_.data(),
      adder1Weights_.size() * sizeof(float));
  RETURN_FALSE_ON_FAILURE(status);

  status = ANeuralNetworksModel_addOperand(model_, &squareFloat32TensorType);
  RETURN_FALSE_ON_FAILURE(status);
  uint32_t adder1Input2OperandId_ = opIdx++;

  // The fuse code passed to the addition is also a constant.
  // Add the operand and set its value.
  status = ANeuralNetworksModel_addOperand(model_, &scalarInt32Type);
  RETURN_FALSE_ON_FAILURE(status);
  uint32_t adder1FusecodeOperandId_ = opIdx++;
  FuseCode fusedActivationCodeValue = ANEURALNETWORKS_FUSED_NONE;
  status = ANeuralNetworksModel_setOperandValue(
      model_, adder1FusecodeOperandId_, &fusedActivationCodeValue,
      sizeof(fusedActivationCodeValue));
  RETURN_FALSE_ON_FAILURE(status);

  status = ANeuralNetworksModel_addOperand(model_, &squareFloat32TensorType);
  RETURN_FALSE_ON_FAILURE(status);
  uint32_t adder1OutputOperandId_ = opIdx++;

  // Add the first Adder
  std::vector<uint32_t> adder1InputOperandIds = {
      adder1Input1OperandId_, adder1Input2OperandId_, adder1FusecodeOperandId_,
  };
  status = ANeuralNetworksModel_addOperation(
      model_, ANEURALNETWORKS_ADD, adder1InputOperandIds.size(),
      adder1InputOperandIds.data(), 1, &adder1OutputOperandId_);
  RETURN_FALSE_ON_FAILURE(status);

  // Add the second Adder and its operands
  status = ANeuralNetworksModel_addOperand(model_, &squareFloat32TensorType);
  RETURN_FALSE_ON_FAILURE(status);
  uint32_t adder2Input1OperandId_ = opIdx++;
  std::fill(adder2Weights_.data(),
            adder2Weights_.data() + dimLength_ * dimLength_,
            kAdder2WeightValue);
  status = ANeuralNetworksModel_setOperandValue(
      model_, adder2Input1OperandId_, adder2Weights_.data(),
      adder2Weights_.size() * sizeof(float));
  RETURN_FALSE_ON_FAILURE(status);

  status = ANeuralNetworksModel_addOperand(model_, &squareFloat32TensorType);
  RETURN_FALSE_ON_FAILURE(status);
  uint32_t adder2Input2OperandId_ = opIdx++;

  status = ANeuralNetworksModel_addOperand(model_, &scalarInt32Type);
  RETURN_FALSE_ON_FAILURE(status);
  uint32_t adder2FusecodeOperandId_ = opIdx++;
  status = ANeuralNetworksModel_setOperandValue(
      model_, adder2FusecodeOperandId_, &fusedActivationCodeValue,
      sizeof(fusedActivationCodeValue));
  RETURN_FALSE_ON_FAILURE(status);

  status = ANeuralNetworksModel_addOperand(model_, &squareFloat32TensorType);
  RETURN_FALSE_ON_FAILURE(status);
  uint32_t adder2OutputOperandId_ = opIdx++;

  std::vector<uint32_t> adder2InputOperandIds = {
      adder2Input1OperandId_,
      adder2Input2OperandId_,
      adder2FusecodeOperandId_,
  };
  status = ANeuralNetworksModel_addOperation(
      model_, ANEURALNETWORKS_ADD, adder2InputOperandIds.size(),
      adder2InputOperandIds.data(), 1, &adder2OutputOperandId_);
  RETURN_FALSE_ON_FAILURE(status);

  status = ANeuralNetworksModel_addOperand(model_, &scalarInt32Type);
  RETURN_FALSE_ON_FAILURE(status);
  uint32_t multiplierFusecodeOperandId_ = opIdx++;
  status = ANeuralNetworksModel_setOperandValue(
      model_, multiplierFusecodeOperandId_, &fusedActivationCodeValue,
      sizeof(fusedActivationCodeValue));
  RETURN_FALSE_ON_FAILURE(status);

  status = ANeuralNetworksModel_addOperand(model_, &squareFloat32TensorType);
  RETURN_FALSE_ON_FAILURE(status);
  uint32_t multiplierOutputOperandId_ = opIdx++;

  // Add the only Multiplier
  std::vector<uint32_t> mulInputOperandIds = {
      adder1OutputOperandId_,  // Adder1's output is input1 for the Multiplier
      adder2OutputOperandId_,  // Adder2's output is input2 for the Multiplier
      multiplierFusecodeOperandId_};
  status = ANeuralNetworksModel_addOperation(
      model_, ANEURALNETWORKS_MUL, mulInputOperandIds.size(),
      mulInputOperandIds.data(), 1, &multiplierOutputOperandId_);
  RETURN_FALSE_ON_FAILURE(status);

  // Identify the operands that are the inputs and outputs of the model.
  //
  // The order of the operands in the input and output lists matters. When users of the
  // model set each input and output values, they identify which value they are setting
  // by specifying an index in the corresponding array.  In this case, as we have two
  // inputs and one output, we will have calls to ANeuralNetworksExecution_setInput with
  // indexes of 0 and 1, and one call to ANeuralNetworksExecution_setOutput with a value of 0.
  // The API is designed this way so that users of the model do not need to know the
  // internal structure of the model.
  std::vector<uint32_t> modelInputOperandIds = {
      adder1Input2OperandId_, adder2Input2OperandId_,
  };
  status = ANeuralNetworksModel_identifyInputsAndOutputs(
      model_, modelInputOperandIds.size(), modelInputOperandIds.data(), 1,
      &multiplierOutputOperandId_);
  RETURN_FALSE_ON_FAILURE(status);

  status = ANeuralNetworksModel_finish(model_);
  RETURN_FALSE_ON_FAILURE(status);

  // Compile the model
  status = ANeuralNetworksCompilation_create(model_, &compilation_);
  RETURN_FALSE_ON_FAILURE(status);

  // We prefer to get the answer quickly, as opposed to conserving battery.
  status = ANeuralNetworksCompilation_setPreference(compilation_,
                  ANEURALNETWORKS_PREFER_FAST_SINGLE_ANSWER);
  RETURN_FALSE_ON_FAILURE(status);

  status = ANeuralNetworksCompilation_finish(compilation_);
  RETURN_FALSE_ON_FAILURE(status);
  return true;
}

/**
 * Report whether model is constructed and ready to compute
 * @return true if ready to compute, false otherwise
 */
bool SimpleModel::IsReady() const { return modelReady_; }

/**
 * Compute with the given input data
 * @param modelInputs:
 *    1st   Adder1_InputOperand2
 *    2nd   Adder2_InputOperand2
 * @return  computed result, or 0.0f if there is error.
 */
bool SimpleModel::Compute(float inputValue1, float inputValue2,
                           float *result) {
  if(!result) {
    return false;
  }
  // Create an execution (an instance of model):
  //   all data go to execution of the model
  //   multiple execution instances could be created from the same compiled
  //   model.
  // This sample only uses one execution of the compiled model.
  ANeuralNetworksExecution* execution;
  int32_t status = ANeuralNetworksExecution_create(compilation_, &execution);
  RETURN_FALSE_ON_FAILURE(status);

  // Set all the elements of the first input tensor to the same value.
  // It's not a realistic example but it shows how to pass a small tensor
  // to an execution.
  std::fill(inputTensor1_.data(), inputTensor1_.data() + tensorSize_,
            inputValue1);

  // Tell the execution to associate inputTensor1 to the first of the two
  // model inputs.
  status = ANeuralNetworksExecution_setInput(execution, 0, nullptr,
                                     inputTensor1_.data(),
                                     tensorSize_ * sizeof(float));
  RETURN_FALSE_ON_FAILURE(status);

  // Do the same thing for the second model input.
  std::fill(inputTensor2_.data(), inputTensor2_.data() + tensorSize_,
            inputValue2);
  status = ANeuralNetworksExecution_setInput(execution, 1, nullptr,
                                             inputTensor2_.data(),
                                             tensorSize_ * sizeof(float));
  RETURN_FALSE_ON_FAILURE(status);

  // Set the output tensor that will be filled by executing the model.
  status = ANeuralNetworksExecution_setOutput(execution, 0, nullptr,
                                              outputTensor_.data(),
                                              tensorSize_ * sizeof(float));
  RETURN_FALSE_ON_FAILURE(status);

  // Start the execution of the model.
  ANeuralNetworksEvent* event = nullptr;
  status = ANeuralNetworksExecution_startCompute(execution, &event);
  RETURN_FALSE_ON_FAILURE(status);

  // Wait until the completion of the execution. This could be done on a different
  // thread. By waiting immediately, we effectively make this a synchronous call.
  status = ANeuralNetworksEvent_wait(event);
  RETURN_FALSE_ON_FAILURE(status);

  ANeuralNetworksEvent_free(event);
  ANeuralNetworksExecution_free(execution);

  for (int32_t idx = 0; idx < tensorSize_; idx++) {
    float delta = outputTensor_[idx] - outputTensor_[0];
    delta = (delta < 0.0f) ? (-delta) : delta;
    if (delta > FLOAT_EPISILON) {
      LOGE("Output computation Error: output0(%f), delta(%f) @ idx(%d)",
           outputTensor_[0], delta, idx);
    }
  }
  *result = outputTensor_[0];
  return result;
}

/*
 * Hardcode a constant value for Adder1Input1Operand and Adder2Input1Operand
 */
float SimpleModel::getAdderConstant(uint32_t adderIdx) {
  return (adderIdx) ? kAdder2WeightValue : kAdder1WeightValue;
}

/**
 * release NNAPI objects
 * @return none
 */
SimpleModel::~SimpleModel() {
  ANeuralNetworksCompilation_free(compilation_);
  ANeuralNetworksModel_free(model_);
}
