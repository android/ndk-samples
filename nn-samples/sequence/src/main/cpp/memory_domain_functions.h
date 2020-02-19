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

NN_MEMORY_DOMAIN_FUNCTION(ANeuralNetworksMemoryDesc_create);
NN_MEMORY_DOMAIN_FUNCTION(ANeuralNetworksMemoryDesc_free);
NN_MEMORY_DOMAIN_FUNCTION(ANeuralNetworksMemoryDesc_addInputRole);
NN_MEMORY_DOMAIN_FUNCTION(ANeuralNetworksMemoryDesc_addOutputRole);
NN_MEMORY_DOMAIN_FUNCTION(ANeuralNetworksMemoryDesc_finish);
NN_MEMORY_DOMAIN_FUNCTION(ANeuralNetworksMemoryDesc_setDimensions);
NN_MEMORY_DOMAIN_FUNCTION(ANeuralNetworksMemory_createFromDesc);
NN_MEMORY_DOMAIN_FUNCTION(ANeuralNetworksMemory_copy);
