/*
 * Copyright 2015 The Android Open Source Project
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
#ifndef NATIVE_AUDIO_DEBUG_UTILS_H
#define NATIVE_AUDIO_DEBUG_UTILS_H
#include <cstdio>
#include <mutex>
#include <string>


/*
 * debug_write_file()
 *  Write given data to a file as binary file. File name is
 *     "/sdcard/data/audio_%d", file_index++
 *  requirement: must have /sdcard/data already created on android device
 */
class Lock {
public:
    explicit Lock(std::recursive_mutex* mtx) {
        mutex_ = mtx;
        mutex_->lock();
    }
    ~Lock() {
        mutex_->unlock();
    }
private:
    std::recursive_mutex  *mutex_;
};
class AndroidLog {
public:
    AndroidLog();
    AndroidLog(std::string &fileName);
    ~AndroidLog();
    void log(void* buf, uint32_t size);
    void log(const char* fmt, ...);
    void logTime();
    void flush();
    static volatile uint32_t fileIdx_;
private:
    uint64_t getCurrentTicks();
    FILE*   fp_;
    FILE*   openFile();
    uint64_t prevTick_;    //Tick in milisecond
    std::recursive_mutex  mutex_;
    std::string  fileName_;
};

void debug_write_file(void* buf, uint32_t size);

#endif //NATIVE_AUDIO_DEBUG_UTILS_H
