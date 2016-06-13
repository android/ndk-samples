/*
 * Copyright (C) 2016 The Android Open Source Project
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
 *
 */
#include <android/log.h>
#include "gmath.h"

#if defined(__GNUC__) && __GNUC__ >= 4
#define GMATH_EXPORT __attribute__((visibility("default")))
#elif defined(__SUNPRO_C) && (__SUNPRO_C >= 0x590)
#define GMATH_EXPORT __attribute__((visibility("default")))
#else
#define GMATH_EXPORT
#endif

#define LOGE(...) \
  ((void)__android_log_print(ANDROID_LOG_ERROR, "gmath::", __VA_ARGS__))

/*
 * return 2 ^ n with multiplication implementation
 */
GMATH_EXPORT unsigned gpower(unsigned n) {
    if (n == 0)
        return 1;
    if (n > 31) {
        LOGE("error from power(%d): integer overflow", n);
        return 0;
    }
    unsigned val = gpower(n>>1) * gpower (n>>1);
    if (n & 1)
      val *= 2;
    return val;
}

