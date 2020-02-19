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

#ifndef NN_SAMPLE_APP_SRC_MAIN_CPP_NEURALNETWORKS_WRAPPER_H_
#define NN_SAMPLE_APP_SRC_MAIN_CPP_NEURALNETWORKS_WRAPPER_H_

#include <android/NeuralNetworks.h>

#define LOG_TAG "NNAPI_SEQUENCE"

#ifdef __cplusplus
extern "C" {
#endif

/**
 *
 * ANeuralNetworksMemoryDesc is an opaque type that represents a memory descriptor.
 * A memory descriptor describes the properties of a memory object, and is used by
 * {@link ANeuralNetworksMemory_createFromDesc}.
 *
 * To use:
 *   - Create a new memory descriptor by calling {@link ANeuralNetworksMemoryDesc_create}.
 *   - Specify all of the intended input and output roles by calling
 *     {@link ANeuralNetworksMemoryDesc_addInputRole} and
 *     {@link ANeuralNetworksMemoryDesc_addOutputRole}.
 *   - Optionally, specify the memory dimensions by calling
 *     {@link ANeuralNetworksMemoryDesc_setDimensions}.
 *   - Complete the memory descriptor with {@link ANeuralNetworksMemoryDesc_finish}.
 *   - Use the memory descriptor as many times as needed with
 *     {@link ANeuralNetworksMemory_createFromDesc}.
 *   - Destroy the memory descriptor with {@link ANeuralNetworksMemoryDesc_free}.
 *
 * A memory descriptor is completed by calling {@link ANeuralNetworksMemoryDesc_finish}.
 * A memory descriptor is destroyed by calling {@link ANeuralNetworksMemoryDesc_free}.
 *
 * A memory descriptor must not be modified once {@link ANeuralNetworksMemoryDesc_finish}
 * has been called on it.
 *
 * It is the application's responsibility to make sure that only
 * one thread modifies a memory descriptor at a given time. It is however
 * safe for more than one thread to use the memory descriptor once
 * {@link ANeuralNetworksMemoryDesc_finish} has returned.
 *
 * It is also the application's responsibility to ensure that there are no other
 * uses of the memory descriptor after calling {@link ANeuralNetworksMemoryDesc_free}.
 * It is however safe to continue using a {@link ANeuralNetworksMemory} object created
 * from the memory descriptor.
 *
 * Available since API level 30.
 */
typedef struct ANeuralNetworksMemoryDesc ANeuralNetworksMemoryDesc;

/**
 * Create a {@link ANeuralNetworksMemoryDesc} with no properties.
 *
 * This only creates the memory descriptor. Its properties should be set with calls to
 * {@link ANeuralNetworksMemoryDesc_addInputRole},
 * {@link ANeuralNetworksMemoryDesc_addOutputRole}, and
 * {@link ANeuralNetworksMemoryDesc_setDimensions}.
 *
 * {@link ANeuralNetworksMemoryDesc_finish} must be called once all properties have been set.
 *
 * {@link ANeuralNetworksMemoryDesc_free} must be called once the memory descriptor
 * is no longer needed.
 *
 * Available since API level 30.
 *
 * @param desc The {@link ANeuralNetworksMemoryDesc} to be created.
 *             Set to NULL if unsuccessful.
 *
 * @return ANEURALNETWORKS_NO_ERROR if successful.
 */
typedef int (*PFN_ANeuralNetworksMemoryDesc_create)(ANeuralNetworksMemoryDesc** desc);
extern PFN_ANeuralNetworksMemoryDesc_create ANeuralNetworksMemoryDesc_create __INTRODUCED_IN(30);

/**
 * Destroy a memory descriptor.
 *
 * The memory descriptor need not have been finished by a call to
 * {@link ANeuralNetworksMemoryDesc_finish}.
 *
 * See {@link ANeuralNetworksMemoryDesc} for information on multithreaded usage.
 *
 * Available since API level 30.
 *
 * @param desc The memory descriptor to be destroyed. Passing NULL is acceptable and
 *             results in no operation.
 */
typedef void (*PFN_ANeuralNetworksMemoryDesc_free)(ANeuralNetworksMemoryDesc* desc);
extern PFN_ANeuralNetworksMemoryDesc_free ANeuralNetworksMemoryDesc_free __INTRODUCED_IN(30);

/**
 * Specify that a memory object will be playing the role of an input to an execution created from a
 * particular compilation.
 *
 * The compilation and the input index fully specify an input operand. This function
 * may be invoked multiple times on the same memory descriptor with different input operands,
 * and the same input operand may be specified on multiple memory descriptors. However,
 * specifying the same input operand on the same memory descriptor more than once will
 * return an error.
 *
 * The dimensions of the corresponding model operands of all the roles specified by
 * {@link ANeuralNetworksMemoryDesc_addInputRole} and
 * {@link ANeuralNetworksMemoryDesc_addOutputRole} must be compatible with each other. Two
 * dimensions are incompatible if both ranks are fully specified but have different values, or if
 * there is at least one axis that is fully specified in both but has different values.
 *
 * At least one of {@link ANeuralNetworksMemoryDesc_addInputRole} and
 * {@link ANeuralNetworksMemoryDesc_addOutputRole} must be called on a memory descriptor
 * before invoking {@link ANeuralNetworksMemoryDesc_finish}.
 *
 * Attempting to modify a memory descriptor once {@link ANeuralNetworksMemoryDesc_finish} has been
 * called will return an error.
 *
 * See {@link ANeuralNetworksMemoryDesc} for information on multithreaded usage.
 *
 * Available since API level 30.
 *
 * @param desc The memory descriptor to be modified.
 * @param compilation The compilation object. It must already have been finished by calling
 *                    {@link ANeuralNetworksCompilation_finish}, and must outlive the memory
 *                    descriptor.
 * @param index The index of the input argument we are referencing from the compilation. It is
 *              an index into the inputs list passed to
 *              {@link ANeuralNetworksModel_identifyInputsAndOutputs}. It is not
 *              the index associated with {@link ANeuralNetworksModel_addOperand}.
 * @param frequency A floating-point value within the range (0.0, 1.0]. Describes how likely the
 *                  memory is to be used in the specified role. This is provided as a hint to
 *                  optimize the case when different roles prefer different memory locations or data
 *                  layouts.
 *
 * @return ANEURALNETWORKS_NO_ERROR if successful.
 */
typedef int (*PFN_ANeuralNetworksMemoryDesc_addInputRole)(
        ANeuralNetworksMemoryDesc* desc, const ANeuralNetworksCompilation* compilation,
        uint32_t index, float frequency);
extern PFN_ANeuralNetworksMemoryDesc_addInputRole ANeuralNetworksMemoryDesc_addInputRole
        __INTRODUCED_IN(30);

/**
 * Specify that a memory object will be playing the role of an output to an execution created from a
 * particular compilation.
 *
 * The compilation and the output index fully specify an output operand. This function
 * may be invoked multiple times on the same memory descriptor with different output operands,
 * and the same output operand may be specified on multiple memory descriptors. However,
 * specifying the same output operand on the same memory descriptor object more than once will
 * return an error.
 *
 * The dimensions of the corresponding model operands of all the roles specified by
 * {@link ANeuralNetworksMemoryDesc_addInputRole} and
 * {@link ANeuralNetworksMemoryDesc_addOutputRole} must be compatible with each other. Two
 * dimensions are incompatible if both ranks are fully specified but have different values, or if
 * there is at least one axis that is fully specified in both but has different values.
 *
 * At least one of {@link ANeuralNetworksMemoryDesc_addInputRole} and
 * {@link ANeuralNetworksMemoryDesc_addOutputRole} must be called on the memory descriptor
 * before invoking {@link ANeuralNetworksMemoryDesc_finish}.
 *
 * Attempting to modify a memory descriptor once {@link ANeuralNetworksMemoryDesc_finish} has been
 * called will return an error.
 *
 * See {@link ANeuralNetworksMemoryDesc} for information on multithreaded usage.
 *
 * Available since API level 30.
 *
 * @param desc The memory descriptor to be modified.
 * @param compilation The compilation object. It must already have been finished by calling
 *                    {@link ANeuralNetworksCompilation_finish}, and must outlive the memory
 *                    descriptor.
 * @param index The index of the output argument we are referencing from the compilation. It is
 *              an index into the outputs list passed to
 *              {@link ANeuralNetworksModel_identifyInputsAndOutputs}. It is not
 *              the index associated with {@link ANeuralNetworksModel_addOperand}.
 * @param frequency A floating-point value within the range (0.0, 1.0]. Describes how likely the
 *                  memory is to be used in the specified role. This is provided as a hint to
 *                  optimize the case when multiple roles prefer different memory locations or data
 *                  layouts.
 *
 * @return ANEURALNETWORKS_NO_ERROR if successful.
 */
typedef int (*PFN_ANeuralNetworksMemoryDesc_addOutputRole)(
        ANeuralNetworksMemoryDesc* desc, const ANeuralNetworksCompilation* compilation,
        uint32_t index, float frequency);
extern PFN_ANeuralNetworksMemoryDesc_addOutputRole ANeuralNetworksMemoryDesc_addOutputRole
        __INTRODUCED_IN(30);

/**
 * Set the dimensional information of the memory descriptor.
 *
 * The specified dimensions must be compatible with the dimensions of the corresponding model
 * operands of all the roles specified by {@link ANeuralNetworksMemoryDesc_addInputRole} and
 * {@link ANeuralNetworksMemoryDesc_addOutputRole}. Two dimensions are incompatible if both ranks
 * are fully specified but have different values, or if there is at least one axis that is fully
 * specified in both but has different values.
 *
 * Attempting to modify a memory descriptor once {@link ANeuralNetworksMemoryDesc_finish} has been
 * called will return an error.
 *
 * See {@link ANeuralNetworksMemoryDesc} for information on multithreaded usage.
 *
 * Available since API level 30.
 *
 * @param desc The memory descriptor to be modified.
 * @param rank The number of dimensions. Must be 0 for scalars.
 * @param dimensions An array of dimensions. An entry with the value 0 indicates that the
 *                   corresponding axis has an unknown size.
 *
 * @return ANEURALNETWORKS_NO_ERROR if successful.
 */
typedef int (*PFN_ANeuralNetworksMemoryDesc_setDimensions)(ANeuralNetworksMemoryDesc* desc,
                                                           uint32_t rank,
                                                           const uint32_t* dimensions);
extern PFN_ANeuralNetworksMemoryDesc_setDimensions ANeuralNetworksMemoryDesc_setDimensions
        __INTRODUCED_IN(30);

/**
 * Indicate that we have finished modifying a memory descriptor. Required before calling
 * {@link ANeuralNetworksMemory_createFromDesc}.
 *
 * This function must only be called once for a given memory descriptor.
 *
 * See {@link ANeuralNetworksMemoryDesc} for information on multithreaded usage.
 *
 * Available since API level 30.
 *
 * @param desc The memory descriptor to be finished.
 *
 * @return ANEURALNETWORKS_NO_ERROR if successful.
 */
typedef int (*PFN_ANeuralNetworksMemoryDesc_finish)(ANeuralNetworksMemoryDesc* desc);
extern PFN_ANeuralNetworksMemoryDesc_finish ANeuralNetworksMemoryDesc_finish __INTRODUCED_IN(30);

/**
 * Creates a memory object from a memory descriptor.
 *
 * The memory object is created with an uninitialized buffer. A memory object with an uninitialized
 * buffer may only be used according to the roles specified by {@link
 * ANeuralNetworksMemoryDesc_addOutputRole}, or as the destination memory in {@link
 * ANeuralNetworksMemory_copy}. The buffer of a memory object is initialized after the memory object
 * is used as an output in a successful execution, or used as the destination memory in a successful
 * {@link ANeuralNetworksMemory_copy}. A memory object with an initialized buffer may be used
 * according to all roles specified in {@link ANeuralNetworksMemoryDesc}, or as the source or
 * destination memory in {@link ANeuralNetworksMemory_copy}. The buffer of a memory object will
 * return to the uninitialized state if the memory object is used as an output in a failed
 * execution, or used as the destination memory in a failed {@link ANeuralNetworksMemory_copy}.
 *
 * The dimensions of the memory descriptor are deduced from the dimensions of the corresponding
 * model operands of all the roles specified by {@link ANeuralNetworksMemoryDesc_addInputRole} and
 * {@link ANeuralNetworksMemoryDesc_addOutputRole}, as well as the dimensions set by the call to
 * {@link ANeuralNetworksMemoryDesc_setDimensions}, if any. The memory descriptor may have
 * unspecified dimensions or rank. In such a case, the same memory object may be used with different
 * shapes of outputs in different executions. When the memory is used as an input, the input shape
 * must be the same as the output shape from the last execution using this memory object as an
 * output, or the last {@link ANeuralNetworkMemory_copy} using this memory object as the destination
 * memory. Creating a memory object with unspecified dimensions or rank may fail for certain sets of
 * roles.
 *
 * Using the memory in roles or shapes that are not compatible with the rules specified above will
 * return an error.
 *
 * When calling {@link ANeuralNetworksExecution_setInputFromMemory} or
 * {@link ANeuralNetworksExecution_setOutputFromMemory} with the memory object,
 * both offset and length must be set to zero and the entire memory region will be
 * associated with the specified input or output operand.
 *
 * Calling {@link ANeuralNetworksModel_setOperandValueFromMemory} with the memory created from this
 * function will return an error.
 *
 * {@link ANeuralNetworksMemory_free} must be called once the memory is no longer needed.
 *
 * Attempting to create memory from an unfinished memory descriptor will return an error.
 *
 * The provided {@link ANeuralNetworksMemoryDesc} need not outlive the {@link ANeuralNetworksMemory}
 * object.
 *
 * Available since API level 30.
 *
 * @param desc The memory descriptor.
 * @param memory The memory object to be created.
 *               Set to NULL if unsuccessful.
 *
 * @return ANEURALNETWORKS_NO_ERROR if successful; ANEURALNETWORKS_OP_FAILED if the memory is
 *         created with unspecified dimensions or rank and it is not supported for this set of
 *         roles.
 */
typedef int (*PFN_ANeuralNetworksMemory_createFromDesc)(const ANeuralNetworksMemoryDesc* desc,
                                                        ANeuralNetworksMemory** memory);
extern PFN_ANeuralNetworksMemory_createFromDesc ANeuralNetworksMemory_createFromDesc
        __INTRODUCED_IN(30);

/**
 * Copies data from one memory object to another.
 *
 * If at most one of the src and dst is created from {@link ANeuralNetworksMemory_createFromDesc},
 * the src and dst must have the same logical size:
 * - If the memory is created from {@link ANeuralNetworksMemory_createFromFd}, or if it is created
 *   from {@link ANeuralNetworksMemory_createFromAHardwareBuffer} with format of
 *   AHARDWAREBUFFER_FORMAT_BLOB, the logical size equals the size of the memory.
 * - If the memory is created from {@link ANeuralNetworksMemory_createFromAHardwareBuffer} with a
 *   format other than AHARDWAREBUFFER_FORMAT_BLOB, the logical size equals the size when there is
 *   no padding and the data is tightly packed. This function may fail if the AHardwareBuffer
 *   cannot be accessed.
 * - If the memory is created from {@link ANeuralNetworksMemory_createFromDesc}, the logical size
 *   equals the size indicated by the {@link OperandCode} multiplied by the number of elements. This
 *   function will fail if the number of elements is unknown.
 *
 * If both src and dst are created from {@link ANeuralNetworksMemory_createFromDesc}, they must have
 * compatible dimensions. Two dimensions are incompatible if both ranks are fully specified but
 * have different values, or if there is at least one axis that is fully specified in both but has
 * different values. The dst may have unspecified dimensions or rank. In such a case, the dimensions
 * of dst will get updated according to the dimensions of the src.
 *
 * In both cases, if the src is created from {@link ANeuralNetworksMemory_createFromDesc}, it must
 * have been used as an output in a successful execution, or used as the destination memory in a
 * successful {@link ANeuralNetworksMemory_copy}.
 *
 * The src and dst may have different data layout, in which case the data copying is performed
 * logically with data layout transformation.
 *
 * Available since API level 30.
 *
 * @param src The source memory object.
 * @param dst The destination memory object.
 *
 * @return ANEURALNETWORKS_NO_ERROR if successful.
 */
typedef int (*PFN_ANeuralNetworksMemory_copy)(const ANeuralNetworksMemory* src,
                                              const ANeuralNetworksMemory* dst);
extern PFN_ANeuralNetworksMemory_copy ANeuralNetworksMemory_copy __INTRODUCED_IN(30);

int InitNeuralNetworks(void);

#ifdef __cplusplus
}
#endif

#endif  // NN_SAMPLE_APP_SRC_MAIN_CPP_NEURALNETWORKS_WRAPPER_H_
