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
#include <time.h>
#include "gperf.h"

#if defined(__GNUC__) && __GNUC__ >= 4
#define GPERF_EXPORT __attribute__((visibility("default")))
#elif defined(__SUNPRO_C) && (__SUNPRO_C >= 0x590)
#define GPERF_EXPORT __attribute__((visibility("default")))
#else
#define GPERF_EXPORT
#endif

/*
 * return current ticks
 */
GPERF_EXPORT uint64_t GetTicks(void) {
    struct timeval Time;
    uint64_t cur_tick = (uint64_t) 1000000;

    gettimeofday( &Time, NULL );
    cur_tick *= Time.tv_sec;

    return (cur_tick + Time.tv_usec);
}

