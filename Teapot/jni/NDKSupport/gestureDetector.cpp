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
// gestureDetector.cpp
//--------------------------------------------------------------------------------

//--------------------------------------------------------------------------------
// includes
//--------------------------------------------------------------------------------
#include "gestureDetector.h"

//--------------------------------------------------------------------------------
// GestureDetector
//--------------------------------------------------------------------------------
GestureDetector::GestureDetector()
{
    _fDpFactor = 1.f;
}

void GestureDetector::setConfiguration(AConfiguration* config)
{
    _fDpFactor = 160.f / AConfiguration_getDensity(config);
}

//--------------------------------------------------------------------------------
// TapDetector
//--------------------------------------------------------------------------------
bool TapDetector::detect(const AInputEvent* motion_event)
{
    if( AMotionEvent_getPointerCount(motion_event) > 1 )
    {
        //Only support single touch
        return false;
    }

    int32_t iAction = AMotionEvent_getAction(motion_event);
    unsigned int flags = iAction & AMOTION_EVENT_ACTION_MASK;
    switch( flags )
    {
    case AMOTION_EVENT_ACTION_DOWN:
        _iDownPointerID = AMotionEvent_getPointerId(motion_event, 0);
        _fDownX = AMotionEvent_getX(motion_event, 0);
        _fDownY = AMotionEvent_getY(motion_event, 0);
        break;
    case AMOTION_EVENT_ACTION_UP:
    {
        int64_t eventTime = AMotionEvent_getEventTime(motion_event);
        int64_t downTime = AMotionEvent_getDownTime(motion_event);
        if( eventTime - downTime <= TAP_TIMEOUT )
        {
            if( _iDownPointerID == AMotionEvent_getPointerId(motion_event, 0) )
            {
                float fX = AMotionEvent_getX(motion_event, 0) - _fDownX;
                float fY = AMotionEvent_getY(motion_event, 0) - _fDownY;
                if( fX * fX + fY * fY < TOUCH_SLOP * TOUCH_SLOP * _fDpFactor )
                {
                    LOGI("TapDetector: Tap detected");
                    return true;
                }
            }
        }
        break;
    }
    }
    return false;
}

//--------------------------------------------------------------------------------
// DoubletapDetector
//--------------------------------------------------------------------------------
bool DoubletapDetector::detect(const AInputEvent* motion_event)
{
    if( AMotionEvent_getPointerCount(motion_event) > 1 )
    {
        //Only support single touch
        return false;
    }

    bool bDetectedTap = _tapDetector.detect(motion_event);

    int32_t iAction = AMotionEvent_getAction(motion_event);
    unsigned int flags = iAction & AMOTION_EVENT_ACTION_MASK;
    switch( flags )
    {
    case AMOTION_EVENT_ACTION_DOWN:
    {
        int64_t eventTime = AMotionEvent_getEventTime(motion_event);
        if( eventTime - _lastTapTime <= DOUBLE_TAP_TIMEOUT )
        {
            float fX = AMotionEvent_getX(motion_event, 0) - _fLastTapX;
            float fY = AMotionEvent_getY(motion_event, 0) - _fLastTapY;
            if( fX * fX + fY * fY < DOUBLE_TAP_SLOP * DOUBLE_TAP_SLOP * _fDpFactor )
            {
                LOGI("DoubletapDetector: Doubletap detected");
                return true;
            }
        }
        break;
    }
    case AMOTION_EVENT_ACTION_UP:
        if( bDetectedTap )
        {
            _lastTapTime = AMotionEvent_getEventTime(motion_event);
            _fLastTapX = AMotionEvent_getX(motion_event, 0);
            _fLastTapY = AMotionEvent_getY(motion_event, 0);
        }
        break;
    }
    return false;
}

void DoubletapDetector::setConfiguration(AConfiguration* config)
{
    _fDpFactor = 160.f / AConfiguration_getDensity(config);
    _tapDetector.setConfiguration(config);
}
