/*
 * Copyright (C) Google Inc.
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
#include <cstdlib>
#include <ctime>

#include "util.hpp"

int Random(int uboundExclusive) {
    int r = rand();
    return r % uboundExclusive;
}

int Random(int lbound, int uboundExclusive) {
    int r = rand();
    r = r % (uboundExclusive - lbound);
    return lbound + r;
}

float Clock() {
    static struct timespec _base;
    static bool firstCall = true;

    if (firstCall) {
        clock_gettime(CLOCK_MONOTONIC, &_base);
        firstCall = false;
    }

    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    float secDiff = (float)(t.tv_sec - _base.tv_sec);
    float msecDiff = (float)((t.tv_nsec - _base.tv_nsec) / 1000000);
    return secDiff + 0.001f * msecDiff;
}


float SineWave(float min, float max, float period, float phase) {
    float ampl = max - min;
    return min + ampl * sin(((Clock() / period) + phase) * 2 * M_PI);
}

bool BlinkFunc(float period) {
    return (int)(Clock() / period) & 1;
}

