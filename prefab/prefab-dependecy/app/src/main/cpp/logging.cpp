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

#include "logging.h"

#include <jni.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

namespace jsonparse::logging {

[[noreturn, gnu::format(printf, 2, 3)]] void FatalError(JNIEnv* env,
                                                        const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  char* msg = nullptr;
  vasprintf(&msg, fmt, ap);
  va_end(ap);

  env->FatalError(msg);
  // env->FatalError() is specified to not return, but the function is not
  // annotated with the noreturn attribute. abort() just in case.
  abort();
}

}  // namespace com.example.prefabdependency::logging
