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

//--------------------------------------------------------------------------------
// gestureDetector.h
//--------------------------------------------------------------------------------

#ifndef GESTUREDETECTOR_H_
#define GESTUREDETECTOR_H_

#include <android/sensor.h>
#include <android/log.h>
#include <android_native_app_glue.h>
#include <android/native_window_jni.h>
#include "JNIHelper.h"

//--------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------
const int32_t DOUBLE_TAP_TIMEOUT = 300 * 1000000;
const int32_t TAP_TIMEOUT = 180 * 1000000;
const int32_t DOUBLE_TAP_SLOP = 100;
const int32_t TOUCH_SLOP = 8;

class GestureDetector
{
protected:
    float _fDpFactor;
public:
    GestureDetector();
    virtual ~GestureDetector() {}
    virtual void setConfiguration(AConfiguration* config);

    virtual bool detect(const AInputEvent* motion_event) = 0;
};

class TapDetector:public GestureDetector
{
    int32_t _iDownPointerID;
    float _fDownX;
    float _fDownY;
public:
    TapDetector() {}
    virtual ~TapDetector() {}
    virtual bool detect(const AInputEvent* motion_event);
};

class DoubletapDetector:public GestureDetector
{
    TapDetector _tapDetector;
    int64_t _lastTapTime;
    float _fLastTapX;
    float _fLastTapY;

public:
    DoubletapDetector() {}
    virtual ~DoubletapDetector() {}
    virtual bool detect(const AInputEvent* motion_event);
    virtual void setConfiguration(AConfiguration* config);
};


#endif /* GESTUREDETECTOR_H_ */
