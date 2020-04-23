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
#include "sequence_model.h"

#include <android/log.h>
#include <android/sharedmem.h>
#include <sys/mman.h>
#include <unistd.h>

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

/**
 * A helper method to allocate an ASharedMemory region and create an
 * ANeuralNetworksMemory object.
 */
static std::pair<int,
                 ANeuralNetworksMemory*> CreateASharedMemory(const char* name,
                                                             uint32_t size,
                                                             int prot) {
  int fd = ASharedMemory_create(name, size * sizeof(float));

  // Create an ANeuralNetworksMemory object from the corresponding ASharedMemory objects.
  ANeuralNetworksMemory* memory = nullptr;
  int32_t status = ANeuralNetworksMemory_createFromFd(size * sizeof(float),
                                                      prot,
                                                      fd,
                                                      0,
                                                      &memory);
  if (status != ANEURALNETWORKS_NO_ERROR) {
    __android_log_print(ANDROID_LOG_ERROR,
                        LOG_TAG,
                        "ANeuralNetworksMemory_createFromFd failed for %s",
                        name);
    close(fd);
    return {-1, nullptr};
  }

  return {fd, memory};
}

/**
 * A helper method to fill the ASharedMemory region with the given value.
 */
static void fillMemory(int fd, uint32_t size, float value) {
  // Set the values of the memory.
  // In reality, the values in the shared memory region will be manipulated by
  // other modules or processes.
  float* data = reinterpret_cast<float*>(
      mmap(nullptr,
           size * sizeof(float),
           PROT_READ | PROT_WRITE,
           MAP_SHARED,
           fd,
           0));
  std::fill(data, data + size, value);
  munmap(data, size * sizeof(float));
}

/**
 * Factory method of SimpleSequenceModel.
 *
 * Create and initialize the model, compilation, and memories associated
 * with the computation graph.
 *
 * @return A pointer to the created model on success, nullptr otherwise
 */
std::unique_ptr<SimpleSequenceModel> SimpleSequenceModel::Create(float ratio) {
  auto model = std::make_unique<SimpleSequenceModel>(ratio);
  if (model->CreateSharedMemories() && model->CreateModel()
      && model->CreateCompilation() && model->CreateOpaqueMemories()) {
    return model;
  }
  return nullptr;
}

/**
 * SimpleSequenceModel Constructor.
 */
SimpleSequenceModel::SimpleSequenceModel(float ratio) : ratio_(ratio) {}

/**
 * Initialize the shared memory objects. In reality, the values in the shared
 * memory region will be manipulated by other modules or processes.
 *
 * @return true for success, false otherwise
 */
bool SimpleSequenceModel::CreateSharedMemories() {
  // Create ASharedMemory to hold the data for initial state, ratio, and sums.
  std::tie(initialStateFd_, memoryInitialState_) =
      CreateASharedMemory("initialState", tensorSize_, PROT_READ);
  std::tie(ratioFd_, memoryRatio_) =
      CreateASharedMemory("ratio", tensorSize_, PROT_READ);
  std::tie(sumInFd_, memorySumIn_) =
      CreateASharedMemory("sumIn", tensorSize_, PROT_READ | PROT_WRITE);
  std::tie(sumOutFd_, memorySumOut_) =
      CreateASharedMemory("sumOut", tensorSize_, PROT_READ | PROT_WRITE);

  // Initialize the ratio tensor.
  fillMemory(ratioFd_, tensorSize_, ratio_);
  return true;
}

/**
 * Create a graph that consists of two operations: one addition and one
 * multiplication. This graph is used for computing a single step of
 * accumulating a geometric progression.
 *
 *   sumIn ---+
 *            +--- ADD ---> sumOut
 * stateIn ---+
 *            +--- MUL ---> stateOut
 *   ratio ---+
 *
 * The ratio is a constant tensor, defined in the model. It represents the
 * weights that would have been learned during a training process.
 *
 * The sumIn and stateIn are input tensors. Their values will be provided when
 * we execute the model. These values can change from execution to execution.
 *
 * To compute the sum of a geometric progression, the graph will be executed
 * multiple times with inputs and outputs chained together.
 *
 *                 +----------+   +----------+         +----------+
 *   initialSum -->| Simple   |-->| Simple   |-->   -->| Simple   |--> sumOut
 *                 | Sequence |   | Sequence |   ...   | Sequence |
 * initialState -->| Model    |-->| Model    |-->   -->| Model    |--> stateOut
 *                 +----------+   +----------+         +----------+
 *
 * @return true for success, false otherwise
 */
bool SimpleSequenceModel::CreateModel() {
  int32_t status;

  // Create the ANeuralNetworksModel handle.
  status = ANeuralNetworksModel_create(&model_);
  if (status != ANEURALNETWORKS_NO_ERROR) {
    __android_log_print(ANDROID_LOG_ERROR,
                        LOG_TAG,
                        "ANeuralNetworksModel_create failed");
    return false;
  }

  uint32_t dimensions[] = {dimLength_, dimLength_};
  ANeuralNetworksOperandType float32TensorType{
      .type = ANEURALNETWORKS_TENSOR_FLOAT32,
      .dimensionCount = sizeof(dimensions) / sizeof(dimensions[0]),
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

  /**
   * Add operands and operations to construct the model.
   *
   * Operands are implicitly identified by the order in which they are added to the model,
   * starting from 0.
   *
   * These indexes are not returned by the model_addOperand call. The application must
   * manage these values. Here, we use opIdx to do the bookkeeping.
   */
  uint32_t opIdx = 0;

  // We first add the operand for the NONE activation function, and set its
  // value to ANEURALNETWORKS_FUSED_NONE.
  // This constant scalar operand will be used for both ADD and MUL.
  status = ANeuralNetworksModel_addOperand(model_, &scalarInt32Type);
  uint32_t fusedActivationFuncNone = opIdx++;
  if (status != ANEURALNETWORKS_NO_ERROR) {
    __android_log_print(ANDROID_LOG_ERROR,
                        LOG_TAG,
                        "ANeuralNetworksModel_addOperand failed for operand (%d)",
                        fusedActivationFuncNone);
    return false;
  }
  FuseCode fusedActivationCodeValue = ANEURALNETWORKS_FUSED_NONE;
  status = ANeuralNetworksModel_setOperandValue(model_, fusedActivationFuncNone,
                                                &fusedActivationCodeValue,
                                                sizeof(fusedActivationCodeValue));
  if (status != ANEURALNETWORKS_NO_ERROR) {
    __android_log_print(ANDROID_LOG_ERROR,
                        LOG_TAG,
                        "ANeuralNetworksModel_setOperandValue failed for operand (%d)",
                        fusedActivationFuncNone);
    return false;
  }

  // sumIn is one of the user provided input tensors to the trained model.
  // Its value is determined pre-execution.
  status = ANeuralNetworksModel_addOperand(model_, &float32TensorType);
  uint32_t sumIn = opIdx++;
  if (status != ANEURALNETWORKS_NO_ERROR) {
    __android_log_print(ANDROID_LOG_ERROR,
                        LOG_TAG,
                        "ANeuralNetworksModel_addOperand failed for operand (%d)",
                        sumIn);
    return false;
  }

  // stateIn is one of the user provided input tensors to the trained model.
  // Its value is determined pre-execution.
  status = ANeuralNetworksModel_addOperand(model_, &float32TensorType);
  uint32_t stateIn = opIdx++;
  if (status != ANEURALNETWORKS_NO_ERROR) {
    __android_log_print(ANDROID_LOG_ERROR,
                        LOG_TAG,
                        "ANeuralNetworksModel_addOperand failed for operand (%d)",
                        stateIn);
    return false;
  }

  // ratio is a constant tensor that was established during training.
  // We read these values from the corresponding ANeuralNetworksMemory object.
  status = ANeuralNetworksModel_addOperand(model_, &float32TensorType);
  uint32_t ratio = opIdx++;
  if (status != ANEURALNETWORKS_NO_ERROR) {
    __android_log_print(ANDROID_LOG_ERROR,
                        LOG_TAG,
                        "ANeuralNetworksModel_addOperand failed for operand (%d)",
                        ratio);
    return false;
  }
  status = ANeuralNetworksModel_setOperandValueFromMemory(model_,
                                                          ratio,
                                                          memoryRatio_,
                                                          0,
                                                          tensorSize_
                                                              * sizeof(float));
  if (status != ANEURALNETWORKS_NO_ERROR) {
    __android_log_print(
        ANDROID_LOG_ERROR,
        LOG_TAG,
        "ANeuralNetworksModel_setOperandValueFromMemory failed for operand (%d)",
        ratio);
    return false;
  }

  // sumOut is the output of the ADD operation.
  // Its value will be computed during execution.
  status = ANeuralNetworksModel_addOperand(model_, &float32TensorType);
  uint32_t sumOut = opIdx++;
  if (status != ANEURALNETWORKS_NO_ERROR) {
    __android_log_print(ANDROID_LOG_ERROR,
                        LOG_TAG,
                        "ANeuralNetworksModel_addOperand failed for operand (%d)",
                        sumOut);
    return false;
  }

  // stateOut is the output of the MUL operation.
  // Its value will be computed during execution.
  status = ANeuralNetworksModel_addOperand(model_, &float32TensorType);
  uint32_t stateOut = opIdx++;
  if (status != ANEURALNETWORKS_NO_ERROR) {
    __android_log_print(ANDROID_LOG_ERROR,
                        LOG_TAG,
                        "ANeuralNetworksModel_addOperand failed for operand (%d)",
                        stateOut);
    return false;
  }

  // Add the ADD operation.
  std::vector<uint32_t> addInputOperands = {
      sumIn,
      stateIn,
      fusedActivationFuncNone,
  };
  status = ANeuralNetworksModel_addOperation(model_,
                                             ANEURALNETWORKS_ADD,
                                             addInputOperands.size(),
                                             addInputOperands.data(),
                                             1,
                                             &sumOut);
  if (status != ANEURALNETWORKS_NO_ERROR) {
    __android_log_print(ANDROID_LOG_ERROR, LOG_TAG,
                        "ANeuralNetworksModel_addOperation failed for ADD");
    return false;
  }

  // Add the MUL operation.
  std::vector<uint32_t> mulInputOperands = {
      stateIn,
      ratio,
      fusedActivationFuncNone,
  };
  status = ANeuralNetworksModel_addOperation(model_,
                                             ANEURALNETWORKS_MUL,
                                             mulInputOperands.size(),
                                             mulInputOperands.data(),
                                             1,
                                             &stateOut);
  if (status != ANEURALNETWORKS_NO_ERROR) {
    __android_log_print(ANDROID_LOG_ERROR, LOG_TAG,
                        "ANeuralNetworksModel_addOperation failed for MUL");
    return false;
  }

  // Identify the input and output tensors to the model.
  // Inputs: {sumIn, stateIn}
  // Outputs: {sumOut, stateOut}
  std::vector<uint32_t> modelInputs = {
      sumIn,
      stateIn,
  };
  std::vector<uint32_t> modelOutputs = {
      sumOut,
      stateOut,
  };
  status =
      ANeuralNetworksModel_identifyInputsAndOutputs(model_,
                                                    modelInputs.size(),
                                                    modelInputs.data(),
                                                    modelOutputs.size(),
                                                    modelOutputs.data());
  if (status != ANEURALNETWORKS_NO_ERROR) {
    __android_log_print(ANDROID_LOG_ERROR,
                        LOG_TAG,
                        "ANeuralNetworksModel_identifyInputsAndOutputs failed");
    return false;
  }

  // Finish constructing the model.
  // The values of constant operands cannot be altered after
  // the finish function is called.
  status = ANeuralNetworksModel_finish(model_);
  if (status != ANEURALNETWORKS_NO_ERROR) {
    __android_log_print(ANDROID_LOG_ERROR,
                        LOG_TAG,
                        "ANeuralNetworksModel_finish failed");
    return false;
  }
  return true;
}

/**
 * Compile the model.
 *
 * @return true for success, false otherwise
 */
bool SimpleSequenceModel::CreateCompilation() {
  int32_t status;

  // Create the ANeuralNetworksCompilation object for the constructed model.
  status = ANeuralNetworksCompilation_create(model_, &compilation_);
  if (status != ANEURALNETWORKS_NO_ERROR) {
    __android_log_print(ANDROID_LOG_ERROR,
                        LOG_TAG,
                        "ANeuralNetworksCompilation_create failed");
    return false;
  }

  // Set the preference for the compilation_, so that the runtime and drivers
  // can make better decisions.
  // Here we prefer to get the answer quickly, so we choose
  // ANEURALNETWORKS_PREFER_FAST_SINGLE_ANSWER.
  status = ANeuralNetworksCompilation_setPreference(compilation_,
                                                    ANEURALNETWORKS_PREFER_FAST_SINGLE_ANSWER);
  if (status != ANEURALNETWORKS_NO_ERROR) {
    __android_log_print(ANDROID_LOG_ERROR, LOG_TAG,
                        "ANeuralNetworksCompilation_setPreference failed");
    return false;
  }

  // Finish the compilation.
  status = ANeuralNetworksCompilation_finish(compilation_);
  if (status != ANEURALNETWORKS_NO_ERROR) {
    __android_log_print(ANDROID_LOG_ERROR,
                        LOG_TAG,
                        "ANeuralNetworksCompilation_finish failed");
    return false;
  }
  return true;
}

/**
 * Create and initialize the opaque memory objects.
 *
 * Opaque memories are suitable for memories that are internal to NNAPI,
 * e.g. state tensors or intermediate results. Using opaque memories may
 * reduce the data copying and transformation overhead.
 *
 * In this example, only the initial sum, the initial state, and the final sum
 * are interesting to us. We do not need to know the intermediate results. So,
 * we create two pairs of opaque memories for intermediate sums and states.
 *
 * @return true for success, false otherwise
 */
bool SimpleSequenceModel::CreateOpaqueMemories() {
  int32_t status;

  // Create opaque memories for sum tensors.
  // We start from creating a memory descriptor and describing all of the
  // intended memory usages.
  ANeuralNetworksMemoryDesc* sumDesc = nullptr;
  status = ANeuralNetworksMemoryDesc_create(&sumDesc);
  if (status != ANEURALNETWORKS_NO_ERROR) {
    __android_log_print(ANDROID_LOG_ERROR,
                        LOG_TAG,
                        "ANeuralNetworksMemoryDesc_create failed");
    return false;
  }

  // Specify that the state memory will be used as the first input (sumIn)
  // of the compilation. Note that the index "0" here means the first operand
  // of the modelInputs list {sumIn, stateIn}, which means sumIn.
  status =
      ANeuralNetworksMemoryDesc_addInputRole(sumDesc, compilation_, 0, 1.0f);
  if (status != ANEURALNETWORKS_NO_ERROR) {
    __android_log_print(ANDROID_LOG_ERROR, LOG_TAG,
                        "ANeuralNetworksMemoryDesc_addInputRole failed");
    ANeuralNetworksMemoryDesc_free(sumDesc);
    return false;
  }

  // Specify that the state memory will also be used as the first output
  // (sumOut) of the compilation. Note that the index "0" here means the
  // first operand of the modelOutputs list {sumOut, stateOut}, which means
  // sumOut.
  status =
      ANeuralNetworksMemoryDesc_addOutputRole(sumDesc, compilation_, 0, 1.0f);
  if (status != ANEURALNETWORKS_NO_ERROR) {
    __android_log_print(ANDROID_LOG_ERROR, LOG_TAG,
                        "ANeuralNetworksMemoryDesc_addOutputRole failed");
    ANeuralNetworksMemoryDesc_free(sumDesc);
    return false;
  }

  // Finish the memory descriptor.
  status = ANeuralNetworksMemoryDesc_finish(sumDesc);
  if (status != ANEURALNETWORKS_NO_ERROR) {
    __android_log_print(ANDROID_LOG_ERROR,
                        LOG_TAG,
                        "ANeuralNetworksMemoryDesc_finish failed");
    ANeuralNetworksMemoryDesc_free(sumDesc);
    return false;
  }

  // Create two opaque memories from the finished descriptor: one for input
  // and one for output. We will swap the two memories after each single
  // execution step.
  status = ANeuralNetworksMemory_createFromDesc(sumDesc, &memoryOpaqueSumIn_);
  if (status != ANEURALNETWORKS_NO_ERROR) {
    __android_log_print(ANDROID_LOG_ERROR,
                        LOG_TAG,
                        "ANeuralNetworksMemory_createFromDesc failed for sum memory #1");
    ANeuralNetworksMemoryDesc_free(sumDesc);
    return false;
  }
  status = ANeuralNetworksMemory_createFromDesc(sumDesc, &memoryOpaqueSumOut_);
  if (status != ANEURALNETWORKS_NO_ERROR) {
    __android_log_print(ANDROID_LOG_ERROR,
                        LOG_TAG,
                        "ANeuralNetworksMemory_createFromDesc failed for sum memory #2");
    ANeuralNetworksMemoryDesc_free(sumDesc);
    return false;
  }

  // It is safe to free the memory descriptor once all of the memories have
  // been created.
  ANeuralNetworksMemoryDesc_free(sumDesc);

  // Create opaque memories for state tensors.
  // We start from creating a memory descriptor and describing all of the
  // intended memory usages.
  ANeuralNetworksMemoryDesc* stateDesc = nullptr;
  status = ANeuralNetworksMemoryDesc_create(&stateDesc);
  if (status != ANEURALNETWORKS_NO_ERROR) {
    __android_log_print(ANDROID_LOG_ERROR,
                        LOG_TAG,
                        "ANeuralNetworksMemoryDesc_create failed");
    return false;
  }

  // Specify that the state memory will be used as the second input (stateIn)
  // of the compilation. Note that the index "1" here means the second operand
  // of the modelInputs list {sumIn, stateIn}, which means stateIn.
  status =
      ANeuralNetworksMemoryDesc_addInputRole(stateDesc, compilation_, 1, 1.0f);
  if (status != ANEURALNETWORKS_NO_ERROR) {
    __android_log_print(ANDROID_LOG_ERROR, LOG_TAG,
                        "ANeuralNetworksMemoryDesc_addInputRole failed");
    ANeuralNetworksMemoryDesc_free(stateDesc);
    return false;
  }

  // Specify that the state memory will also be used as the second output
  // (stateOut) of the compilation. Note that the index "1" here means the
  // second operand of the modelOutputs list {sumOut, stateOut}, which means
  // stateOut.
  status =
      ANeuralNetworksMemoryDesc_addOutputRole(stateDesc, compilation_, 1, 1.0f);
  if (status != ANEURALNETWORKS_NO_ERROR) {
    __android_log_print(ANDROID_LOG_ERROR, LOG_TAG,
                        "ANeuralNetworksMemoryDesc_addOutputRole failed");
    ANeuralNetworksMemoryDesc_free(stateDesc);
    return false;
  }

  // Finish the memory descriptor.
  status = ANeuralNetworksMemoryDesc_finish(stateDesc);
  if (status != ANEURALNETWORKS_NO_ERROR) {
    __android_log_print(ANDROID_LOG_ERROR,
                        LOG_TAG,
                        "ANeuralNetworksMemoryDesc_finish failed");
    ANeuralNetworksMemoryDesc_free(stateDesc);
    return false;
  }

  // Create two opaque memories from the finished descriptor: one for input
  // and one for output. We will swap the two memories after each single
  // execution step.
  status =
      ANeuralNetworksMemory_createFromDesc(stateDesc, &memoryOpaqueStateIn_);
  if (status != ANEURALNETWORKS_NO_ERROR) {
    __android_log_print(ANDROID_LOG_ERROR,
                        LOG_TAG,
                        "ANeuralNetworksMemory_createFromDesc failed for state memory #1");
    ANeuralNetworksMemoryDesc_free(stateDesc);
    return false;
  }
  status =
      ANeuralNetworksMemory_createFromDesc(stateDesc, &memoryOpaqueStateOut_);
  if (status != ANEURALNETWORKS_NO_ERROR) {
    __android_log_print(ANDROID_LOG_ERROR,
                        LOG_TAG,
                        "ANeuralNetworksMemory_createFromDesc failed for state memory #2");
    ANeuralNetworksMemoryDesc_free(stateDesc);
    return false;
  }

  // It is safe to free the memory descriptor once all of the memories have
  // been created.
  ANeuralNetworksMemoryDesc_free(stateDesc);
  return true;
}

/**
 * Dispatch a single computation step of accumulating the geometric progression.
 */
static bool DispatchSingleStep(ANeuralNetworksCompilation* compilation,
                              ANeuralNetworksMemory* sumIn,
                              uint32_t sumInLength,
                              ANeuralNetworksMemory* stateIn,
                              uint32_t stateInLength,
                              ANeuralNetworksMemory* sumOut,
                              uint32_t sumOutLength,
                              ANeuralNetworksMemory* stateOut,
                              uint32_t stateOutLength,
                              const ANeuralNetworksEvent* waitFor,
                              ANeuralNetworksEvent** event) {
  // Create an ANeuralNetworksExecution object from the compiled model.
  ANeuralNetworksExecution* execution;
  int32_t status = ANeuralNetworksExecution_create(compilation, &execution);
  if (status != ANEURALNETWORKS_NO_ERROR) {
    __android_log_print(ANDROID_LOG_ERROR,
                        LOG_TAG,
                        "ANeuralNetworksExecution_create failed");
    return false;
  }

  // Set the memory for the sumIn tensor.
  // Note that the index "0" here means the first operand of the modelInputs
  // list {sumIn, stateIn}, which means sumIn.
  status = ANeuralNetworksExecution_setInputFromMemory(execution,
                                                       0,
                                                       nullptr,
                                                       sumIn,
                                                       0,
                                                       sumInLength
                                                           * sizeof(float));
  if (status != ANEURALNETWORKS_NO_ERROR) {
    __android_log_print(ANDROID_LOG_ERROR,
                        LOG_TAG,
                        "ANeuralNetworksExecution_setInputFromMemory failed for sumIn");
    return false;
  }

  // Set the memory for the stateIn tensor.
  // Note that the index "1" here means the first operand of the modelInputs
  // list {sumIn, stateIn}, which means stateIn.
  status = ANeuralNetworksExecution_setInputFromMemory(execution,
                                                       1,
                                                       nullptr,
                                                       stateIn,
                                                       0,
                                                       stateInLength
                                                           * sizeof(float));
  if (status != ANEURALNETWORKS_NO_ERROR) {
    __android_log_print(ANDROID_LOG_ERROR,
                        LOG_TAG,
                        "ANeuralNetworksExecution_setInputFromMemory failed for stateIn");
    return false;
  }

  // Set the sumOut tensor that will be filled by executing the model.
  status = ANeuralNetworksExecution_setOutputFromMemory(execution,
                                                        0,
                                                        nullptr,
                                                        sumOut,
                                                        0,
                                                        sumOutLength
                                                            * sizeof(float));
  if (status != ANEURALNETWORKS_NO_ERROR) {
    __android_log_print(ANDROID_LOG_ERROR,
                        LOG_TAG,
                        "ANeuralNetworksExecution_setOutputFromMemory failed for sumOut");
    return false;
  }

  // Set the stateOut tensor that will be filled by executing the model.
  status = ANeuralNetworksExecution_setOutputFromMemory(execution,
                                                        1,
                                                        nullptr,
                                                        stateOut,
                                                        0,
                                                        stateOutLength
                                                            * sizeof(float));
  if (status != ANEURALNETWORKS_NO_ERROR) {
    __android_log_print(ANDROID_LOG_ERROR,
                        LOG_TAG,
                        "ANeuralNetworksExecution_setOutputFromMemory failed for stateOut");
    return false;
  }

  // Dispatch the execution of the model.
  // Note that the execution here is asynchronous with dependencies.
  const ANeuralNetworksEvent* const* dependencies = nullptr;
  uint32_t numDependencies = 0;
  if (waitFor != nullptr) {
    dependencies = &waitFor;
    numDependencies = 1;
  }
  status = ANeuralNetworksExecution_startComputeWithDependencies(execution,
                                                                 dependencies,
                                                                 numDependencies,
                                                                 0,  // infinite timeout duration
                                                                 event);
  if (status != ANEURALNETWORKS_NO_ERROR) {
    __android_log_print(ANDROID_LOG_ERROR, LOG_TAG,
                        "ANeuralNetworksExecution_compute failed");
    return false;
  }

  ANeuralNetworksExecution_free(execution);
  return true;
}

/**
 * Compute the sum of a geometric progression.
 *
 * @param   initialValue  the initial value of the geometric progression
 * @param   steps         the number of terms to accumulate
 * @return  computed result, or 0.0f if there is error.
 */
bool SimpleSequenceModel::Compute(float initialValue,
                                  uint32_t steps,
                                  float* result) {
  if (!result) {
    return false;
  }
  if (steps == 0) {
    *result = 0.0f;
    return true;
  }

  // Setup initial values.
  // In reality, the values in the shared memory region will be manipulated by
  // other modules or processes.
  fillMemory(sumInFd_, tensorSize_, 0);
  fillMemory(initialStateFd_, tensorSize_, initialValue);

  // The event objects for all computation steps.
  std::vector<ANeuralNetworksEvent*> events(steps, nullptr);

  for (uint32_t i = 0; i < steps; i++) {
    // We will only use ASharedMemory for boundary step executions, and use
    // opaque memories for intermediate results to minimize the data copying.
    // Note that when setting an opaque memory as the input or output of an
    // execution, the offset and length must be set to 0 to indicate the
    // entire memory region is used.
    ANeuralNetworksMemory* sumInMemory;
    ANeuralNetworksMemory* sumOutMemory;
    ANeuralNetworksMemory* stateInMemory;
    ANeuralNetworksMemory* stateOutMemory;
    uint32_t sumInLength, sumOutLength, stateInLength, stateOutLength;
    if (i == 0) {
      sumInMemory = memorySumIn_;
      sumInLength = tensorSize_;
      stateInMemory = memoryInitialState_;
      stateInLength = tensorSize_;
    } else {
      sumInMemory = memoryOpaqueSumIn_;
      sumInLength = 0;
      stateInMemory = memoryOpaqueStateIn_;
      stateInLength = 0;
    }
    if (i == steps - 1) {
      sumOutMemory = memorySumOut_;
      sumOutLength = tensorSize_;
    } else {
      sumOutMemory = memoryOpaqueSumOut_;
      sumOutLength = 0;
    }
    stateOutMemory = memoryOpaqueStateOut_;
    stateOutLength = 0;

    // Dispatch a single computation step with a dependency on the previous step, if any.
    // The actual computation will start once its dependency has finished.
    const ANeuralNetworksEvent* waitFor = i == 0 ? nullptr : events[i - 1];
    if (!DispatchSingleStep(compilation_,
                           sumInMemory,
                           sumInLength,
                           stateInMemory,
                           stateInLength,
                           sumOutMemory,
                           sumOutLength,
                           stateOutMemory,
                           stateOutLength,
                           waitFor,
                           &events[i])) {
      __android_log_print(ANDROID_LOG_ERROR,
                          LOG_TAG,
                          "DispatchSingleStep failed for step %d",
                          i);
      return false;
    }

    // Swap the memory handles: the outputs from the current step execution
    // will be fed in as the inputs of the next step execution.
    std::swap(memoryOpaqueSumIn_, memoryOpaqueSumOut_);
    std::swap(memoryOpaqueStateIn_, memoryOpaqueStateOut_);
  }

  // Since the events are chained, we only need to wait for the last one.
  ANeuralNetworksEvent_wait(events.back());

  // Get the results.
  float* outputTensorPtr = reinterpret_cast<float*>(
      mmap(nullptr,
           tensorSize_ * sizeof(float),
           PROT_READ,
           MAP_SHARED,
           sumOutFd_,
           0));
  *result = outputTensorPtr[0];
  munmap(outputTensorPtr, tensorSize_ * sizeof(float));

  // Cleanup event objects.
  for (auto* event : events) {
      ANeuralNetworksEvent_free(event);
  }
  return true;
}

/**
 * SimpleSequenceModel Destructor.
 *
 * Release NN API objects and close the file descriptors.
 */
SimpleSequenceModel::~SimpleSequenceModel() {
  ANeuralNetworksCompilation_free(compilation_);
  ANeuralNetworksModel_free(model_);

  ANeuralNetworksMemory_free(memorySumIn_);
  ANeuralNetworksMemory_free(memorySumOut_);
  ANeuralNetworksMemory_free(memoryInitialState_);
  ANeuralNetworksMemory_free(memoryRatio_);
  close(initialStateFd_);
  close(sumInFd_);
  close(sumOutFd_);
  close(ratioFd_);

  ANeuralNetworksMemory_free(memoryOpaqueStateIn_);
  ANeuralNetworksMemory_free(memoryOpaqueStateOut_);
  ANeuralNetworksMemory_free(memoryOpaqueSumIn_);
  ANeuralNetworksMemory_free(memoryOpaqueSumOut_);
}
