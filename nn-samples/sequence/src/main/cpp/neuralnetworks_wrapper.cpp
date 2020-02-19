/*
 * Copyright (C) 2020 The Android Open Source Project
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

#include "neuralnetworks_wrapper.h"

#include <android/log.h>
#include <dlfcn.h>

int InitNeuralNetworks(void) {
  void* libnn = dlopen("libneuralnetworks.so", RTLD_NOW | RTLD_LOCAL);
  if (!libnn) return 0;

#define NN_MEMORY_DOMAIN_FUNCTION(func)                                          \
  func = (PFN_##func)dlsym(libnn, #func);                                        \
  if (!func) {                                                                   \
      __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, "dlsym %s failed", #func); \
      return 0;                                                                  \
  }
#include "memory_domain_functions.h"
#undef NN_MEMORY_DOMAIN_FUNCTION

  return 1;
}

#define NN_MEMORY_DOMAIN_FUNCTION(func) PFN_##func func;
#include "memory_domain_functions.h"
#undef NN_MEMORY_DOMAIN_FUNCTION
