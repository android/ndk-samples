/*
 * Copyright 2013 The Android Open Source Project
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

#ifndef PERFMONITOR_H_
#define PERFMONITOR_H_

#include <jni.h>
#include <errno.h>
#include <time.h>
#include "JNIHelper.h"

namespace ndk_helper {

const int32_t kNumSamples = 100;

/******************************************************************
 * Helper class for a performance monitoring and get current tick time
 */
class PerfMonitor {
 private:
  float current_FPS_;
  time_t tv_last_sec_;

  double last_tick_;
  int32_t tickindex_;
  double ticksum_;
  double ticklist_[kNumSamples];

  double UpdateTick(double current_tick);

 public:
  PerfMonitor();
  virtual ~PerfMonitor();

  bool Update(float &fFPS);

  static double GetCurrentTime() {
    struct timeval time;
    gettimeofday(&time, NULL);
    double ret = time.tv_sec + time.tv_usec * 1.0 / 1000000.0;
    return ret;
  }
};

}  // namespace ndkHelper
#endif /* PERFMONITOR_H_ */
