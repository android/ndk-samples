/*
 * Copyright (C) 2021 The Android Open Source Project
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
#pragma once

#include <ctime>

/**
 * DisplayTimer: a utility class to track the display time in nano seconds
 */
class DisplayTimer {
public:
    explicit DisplayTimer(void) {
        Reset(0);
    }

    void Reset(uint64_t durationInNanoSecond) {
        durationInNano = durationInNanoSecond;
        struct timespec cur;
        clock_gettime(CLOCKS_MONO, &cur);
        startTime = cur.tv_sec * 1000000000UL + cur.tv_nsec;
    }

    bool IsExpired(void) {
        uint64_t timePassed = GetTimePassed();
        return timePassed >= durationInNano ? true : false;
    }

    uint64_t timeLeft(void) {
        uint64_t timePassed = GetTimePassed();
        return (timePassed >= durationInNano) ? 0 : (timePassed - durationInNano);
    }

private:
    uint64_t GetTimePassed(void) {
        struct timespec cur;
        clock_gettime(CLOCKS_MONO, &cur);
        uint64_t curNS = cur.tv_sec * 1000000000UL + cur.tv_nsec;

        return (curNS - startTime);
    }

    uint64_t durationInNano;
    uint64_t startTime;
};