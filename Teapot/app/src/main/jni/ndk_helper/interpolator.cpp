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

#include "interpolator.h"
#include <math.h>
#include "interpolator.h"

namespace ndk_helper {

//-------------------------------------------------
// Ctor
//-------------------------------------------------
Interpolator::Interpolator() { list_params_.clear(); }

//-------------------------------------------------
// Dtor
//-------------------------------------------------
Interpolator::~Interpolator() { list_params_.clear(); }

void Interpolator::Clear() { list_params_.clear(); }

Interpolator& Interpolator::Set(const float start, const float dest,
                                const INTERPOLATOR_TYPE type,
                                const double duration) {
  // init the parameters for the interpolation process
  start_time_ = PerfMonitor::GetCurrentTime();
  dest_time_ = start_time_ + duration;
  type_ = type;

  start_value_ = start;
  dest_value_ = dest;
  return *this;
}

Interpolator& Interpolator::Add(const float dest, const INTERPOLATOR_TYPE type,
                                const double duration) {
  InterpolatorParams param;
  param.dest_value_ = dest;
  param.type_ = type;
  param.duration_ = duration;
  list_params_.push_back(param);
  return *this;
}

bool Interpolator::Update(const double current_time, float& p) {
  bool bContinue;
  if (current_time >= dest_time_) {
    p = dest_value_;
    if (list_params_.size()) {
      InterpolatorParams& item = list_params_.front();
      Set(dest_value_, item.dest_value_, item.type_, item.duration_);
      list_params_.pop_front();

      bContinue = true;
    } else {
      bContinue = false;
    }
  } else {
    float t = (float)(current_time - start_time_);
    float d = (float)(dest_time_ - start_time_);
    float b = start_value_;
    float c = dest_value_ - start_value_;
    p = GetFormula(type_, t, b, d, c);

    bContinue = true;
  }
  return bContinue;
}

float Interpolator::GetFormula(const INTERPOLATOR_TYPE type, const float t,
                               const float b, const float d, const float c) {
  float t1;
  switch (type) {
    case INTERPOLATOR_TYPE_LINEAR:
      // simple linear interpolation - no easing
      return (c * t / d + b);

    case INTERPOLATOR_TYPE_EASEINQUAD:
      // quadratic (t^2) easing in - accelerating from zero velocity
      t1 = t / d;
      return (c * t1 * t1 + b);

    case INTERPOLATOR_TYPE_EASEOUTQUAD:
      // quadratic (t^2) easing out - decelerating to zero velocity
      t1 = t / d;
      return (-c * t1 * (t1 - 2) + b);

    case INTERPOLATOR_TYPE_EASEINOUTQUAD:
      // quadratic easing in/out - acceleration until halfway, then deceleration
      t1 = t / d / 2;
      if (t1 < 1)
        return (c / 2 * t1 * t1 + b);
      else {
        t1 = t1 - 1;
        return (-c / 2 * (t1 * (t1 - 2) - 1) + b);
      }
    case INTERPOLATOR_TYPE_EASEINCUBIC:
      // cubic easing in - accelerating from zero velocity
      t1 = t / d;
      return (c * t1 * t1 * t1 + b);

    case INTERPOLATOR_TYPE_EASEOUTCUBIC:
      // cubic easing in - accelerating from zero velocity
      t1 = t / d - 1;
      return (c * (t1 * t1 * t1 + 1) + b);

    case INTERPOLATOR_TYPE_EASEINOUTCUBIC:
      // cubic easing in - accelerating from zero velocity
      t1 = t / d / 2;

      if (t1 < 1)
        return (c / 2 * t1 * t1 * t1 + b);
      else {
        t1 -= 2;
        return (c / 2 * (t1 * t1 * t1 + 2) + b);
      }
    case INTERPOLATOR_TYPE_EASEINQUART:
      // quartic easing in - accelerating from zero velocity
      t1 = t / d;
      return (c * t1 * t1 * t1 * t1 + b);

    case INTERPOLATOR_TYPE_EASEINEXPO:
      // exponential (2^t) easing in - accelerating from zero velocity
      if (t == 0)
        return b;
      else
        return (c * powf(2, (10 * (t / d - 1))) + b);

    case INTERPOLATOR_TYPE_EASEOUTEXPO:
      // exponential (2^t) easing out - decelerating to zero velocity
      if (t == d)
        return (b + c);
      else
        return (c * (-powf(2, -10 * t / d) + 1) + b);
    default:
      return 0;
  }
}

}  // namespace ndkHelper
