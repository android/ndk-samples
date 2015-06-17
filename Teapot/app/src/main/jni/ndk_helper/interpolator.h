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

#ifndef INTERPOLATOR_H_
#define INTERPOLATOR_H_

#include <jni.h>
#include <errno.h>
#include <time.h>
#include "JNIHelper.h"
#include "perfMonitor.h"
#include <list>

namespace ndk_helper
{

enum INTERPOLATOR_TYPE
{
    INTERPOLATOR_TYPE_LINEAR,
    INTERPOLATOR_TYPE_EASEINQUAD,
    INTERPOLATOR_TYPE_EASEOUTQUAD,
    INTERPOLATOR_TYPE_EASEINOUTQUAD,
    INTERPOLATOR_TYPE_EASEINCUBIC,
    INTERPOLATOR_TYPE_EASEOUTCUBIC,
    INTERPOLATOR_TYPE_EASEINOUTCUBIC,
    INTERPOLATOR_TYPE_EASEINQUART,
    INTERPOLATOR_TYPE_EASEINEXPO,
    INTERPOLATOR_TYPE_EASEOUTEXPO,
};

struct InterpolatorParams
{
    float dest_value_;
    INTERPOLATOR_TYPE type_;
    double duration_;
};

/******************************************************************
 * Interpolates values with several interpolation methods
 */
class Interpolator
{
private:
    double start_time_;
    double dest_time_;
    INTERPOLATOR_TYPE type_;

    float start_value_;
    float dest_value_;
    std::list<InterpolatorParams> list_params_;

    float GetFormula( const INTERPOLATOR_TYPE type,
            const float t,
            const float b,
            const float d,
            const float c );
public:
    Interpolator();
    ~Interpolator();

    Interpolator& Set( const float start,
            const float dest,
            const INTERPOLATOR_TYPE type,
            double duration );

    Interpolator& Add( const float dest,
            const INTERPOLATOR_TYPE type,
            const double duration );

    bool Update( const double currentTime, float& p );

    void Clear();
};

}   //namespace ndkHelper
#endif /* INTERPOLATOR_H_ */
