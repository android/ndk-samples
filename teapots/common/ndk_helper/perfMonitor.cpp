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

#include "perfMonitor.h"

namespace ndk_helper {

PerfMonitor::PerfMonitor()
    : current_FPS_(0),
      tv_last_sec_(0),
      last_tick_(0.f),
      tickindex_(0),
      ticksum_(0) {
  for (int32_t i = 0; i < kNumSamples; ++i) ticklist_[i] = 0;
}

PerfMonitor::~PerfMonitor() {}

double PerfMonitor::UpdateTick(double currentTick) {
  ticksum_ -= ticklist_[tickindex_];
  ticksum_ += currentTick;
  ticklist_[tickindex_] = currentTick;
  tickindex_ = (tickindex_ + 1) % kNumSamples;

  return ((double)ticksum_ / kNumSamples);
}

bool PerfMonitor::Update(float &fFPS) {
  struct timeval Time;
  gettimeofday(&Time, NULL);

  double time = Time.tv_sec + Time.tv_usec * 1.0 / 1000000.0;
  double tick = time - last_tick_;
  double d = UpdateTick(tick);
  last_tick_ = time;

  if (Time.tv_sec - tv_last_sec_ >= 1) {
    current_FPS_ = 1.f / d;
    tv_last_sec_ = Time.tv_sec;
    fFPS = current_FPS_;
    return true;
  } else {
    fFPS = current_FPS_;
    return false;
  }
}

}  // namespace ndkHelper
