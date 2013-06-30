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

//----------------------------------------------------------
//  tapCamera.cpp
//  Camera control with tap
//
//----------------------------------------------------------

#include <fstream>
#include "tapCamera.h"

const float TRANSFORM_FACTOR = 15.f;
const float TRANSFORM_FACTORZ = 10.f;

const float MOMENTUM_FACTOR_DECREASE = 0.85f;
const float MOMENTUM_FACTOR_DECREASE_SHIFT = 0.9f;
const float MOMENTUM_FACTOR = 0.8f;
const float MOMENTUM_FACTOR_THRESHOLD = 0.001f;

//----------------------------------------------------------
//  Ctor
//----------------------------------------------------------
tapCamera::tapCamera():_bDragging(false), _bMomentum(false), _fBallRadius( 0.75f )
{
    //Init offset
    initParameters();

    _vFlip = vec2( 1.f, -1.f );
    _fFlipZ = -1.f;
}

void tapCamera::initParameters()
{
    //Init parameters
    _vecOffset = vec3();
    _vecOffsetNow = vec3();

    _qBallNow = quaternion();
    _qBallRot = quaternion();
    _bMomentum = false;
}

//----------------------------------------------------------
//  Dtor
//----------------------------------------------------------
tapCamera::~tapCamera()
{

}

void tapCamera::update()
{
    if( _bMomentum )
    {
        float fMomenttumSteps = _fMomentumSteps;

        //Momentum rotation
        vec2 v = _vDragDelta;
        beginDrag(vec2() ); //NOTE:This call reset _VDragDelta
        drag(v * _vFlip);

        //Momentum shift
        _vecOffset += _vecOffsetDelta;

        ballUpdate();
        endDrag();

        //Decrease deltas
        _vDragDelta = v * MOMENTUM_FACTOR_DECREASE;
        _vecOffsetDelta = _vecOffsetDelta * MOMENTUM_FACTOR_DECREASE_SHIFT;

        //Count steps
        _fMomentumSteps = fMomenttumSteps * MOMENTUM_FACTOR_DECREASE;
        if( _fMomentumSteps < MOMENTUM_FACTOR_THRESHOLD )
        {
            _bMomentum = false;
        }
    }
    else
    {
        _vDragDelta *= MOMENTUM_FACTOR;
        _vecOffsetDelta = _vecOffsetDelta * MOMENTUM_FACTOR;
        ballUpdate();
    }

    vec3 vec = _vecOffset + _vecOffsetNow;
    vec3 vecTmp(TRANSFORM_FACTOR, -TRANSFORM_FACTOR, TRANSFORM_FACTORZ);
    vec *= vecTmp;

    _mTransform = mat4::translation(vec);
}

mat4& tapCamera::getRotationMatrix()
{
    return _mRotation;
}

mat4& tapCamera::getTransformMatrix()
{
    return _mTransform;
}

void tapCamera::reset(const bool bAnimate)
{
    initParameters();
}

//----------------------------------------------------------
//Drag control
//----------------------------------------------------------
void tapCamera::beginDrag(const vec2& v)
{
    if( _bDragging )
        endDrag();

    if( _bPinching )
        endPinch();

    vec2 vec = v * _vFlip;
    _vBallNow = vec;
    _vBallDown = _vBallNow;

    _bDragging = true;
    _bMomentum = false;
    _vLastInput = vec;
    _vDragDelta = vec2();
}

void tapCamera::endDrag()
{
    int i;
    _qBallDown = _qBallNow;
    _qBallRot = quaternion();

    _bDragging = false;
    _bMomentum = true;
    _fMomentumSteps = 1.0f;
}

void tapCamera::drag(const vec2& v )
{
    if( !_bDragging )
        return;

    vec2 vec = v * _vFlip;
    _vBallNow = vec;

    _vDragDelta = _vDragDelta * MOMENTUM_FACTOR + (vec - _vLastInput);
    _vLastInput = vec;
}

//----------------------------------------------------------
//Pinch controll
//----------------------------------------------------------
void tapCamera::beginPinch(const vec2& v1, const vec2& v2)
{
    if( _bDragging )
        endDrag();

    if( _bPinching )
        endPinch();

    beginDrag( vec2() );

    _vecPinchStartCenter = (v1 + v2) / 2.f;

    vec2 vec = v1 - v2;
    float fXDiff;
    float fYDiff;
    vec.value( fXDiff, fYDiff );

    _fPinchStartDistanceSQ = fXDiff*fXDiff + fYDiff*fYDiff;
    _fRotationStart = atan2f( fYDiff, fXDiff );
    _fRotationNow = 0;

    _bPinching = true;
    _bMomentum = false;

    //Init momentum factors
    _vecOffsetDelta = vec3();
}


void tapCamera::endPinch()
{
    _bPinching = false;
    _bMomentum = true;
    _fMomentumSteps = 1.f;
    _vecOffset += _vecOffsetNow;
    _fRotation += _fRotationNow;
    _vecOffsetNow = vec3();

    _fRotationNow = 0;

    endDrag();

}

void tapCamera::pinch(const vec2& v1, const vec2& v2)
{
    if( !_bPinching )
        return;

    //Update momentum factor
    _vecOffsetLast = _vecOffsetNow;

    float fXDiff, fYDiff;
    vec2 vec = v1 - v2;
    vec.value(fXDiff, fYDiff);

    float fDistanceSQ = fXDiff * fXDiff + fYDiff * fYDiff;

    vec = (v1 + v2) / 2.f - _vecPinchStartCenter;
    _vecOffsetNow = vec3( vec,
            _fFlipZ * (_fPinchStartDistanceSQ / fDistanceSQ - 1.f) );

    //Update momentum factor
    _vecOffsetDelta = _vecOffsetDelta * MOMENTUM_FACTOR + (_vecOffsetNow - _vecOffsetLast);

    //
    //Update ration quaternion
    float fRotation = atan2f( fYDiff, fXDiff );
    _fRotationNow = fRotation - _fRotationStart;

    //Trackball rotation
    _qBallRot = quaternion( 0.f, 0.f, sinf(-_fRotationNow*0.5f), cosf(-_fRotationNow*0.5f) );
}

//----------------------------------------------------------
//Trackball controll
//----------------------------------------------------------
void tapCamera::ballUpdate()
{
    if (_bDragging) {
        vec3 vFrom = pointOnSphere(_vBallDown);
        vec3 vTo = pointOnSphere(_vBallNow);

        vec3 vec = vFrom.cross(vTo);
        float w = vFrom.dot( vTo );

        quaternion qDrag = quaternion(vec, w);
        qDrag = qDrag * _qBallDown;
        _qBallNow = _qBallRot * qDrag;
    }
    _qBallNow.toMatrix(_mRotation);
}

vec3 tapCamera::pointOnSphere(vec2& point)
{
    vec3 ballMouse;
    float mag;
    vec2 vec = (point - _vBallCenter) / _fBallRadius;
    mag = vec.dot( vec );
    if (mag > 1.f)
    {
        float scale = 1.f / sqrtf(mag);
        vec *= scale;
        ballMouse = vec3( vec, 0.f );
    } else {
        ballMouse = vec3( vec, sqrtf(1.f - mag) );
    }
    return (ballMouse);
}
