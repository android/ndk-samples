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
//  tapCamera.h
//  Camera control with tap
//----------------------------------------------------------
#pragma once
#include <vector>
#include <string>
#include <GLES2/gl2.h>

#include "JNIHelper.h"
#include "vecmath.h"

class tapCamera
{
    //Trackball
    vec2 _vBallCenter;
    float _fBallRadius;
    quaternion _qBallNow, _qBallDown;
    vec2 _vBallNow, _vBallDown;
    quaternion _qBallRot;

    bool _bDragging;
    bool _bPinching;

    //Pinch related info
    vec2 _vecPinchStart;
    vec2 _vecPinchStartCenter;
    float _fPinchStartDistanceSQ;

    //Camera shift
    vec3 _vecOffset;
    vec3 _vecOffsetNow;

    //Camera Rotation
    float _fRotation;
    float _fRotationStart;
    float _fRotationNow;

    //Momentum support
    bool _bMomentum;
    vec2 _vDragDelta;
    vec2 _vLastInput;
    vec3 _vecOffsetLast;
    vec3 _vecOffsetDelta;
    float _fMomentumSteps;

    vec2 _vFlip;
    float _fFlipZ;

    mat4 _mRotation;
    mat4 _mTransform;

    vec3 pointOnSphere(vec2& point);
    void ballUpdate();
    void initParameters();
public:
    tapCamera();
    virtual ~tapCamera();
    void beginDrag(const vec2& vec);
    void endDrag();
    void drag(const vec2& vec);
    void update();

    mat4& getRotationMatrix();
    mat4& getTransformMatrix();

    void beginPinch(const vec2& v1, const vec2& v2);
    void endPinch();
    void pinch(const vec2& v1, const vec2& v2);

    void setFlip(const float fX, const float fY, const float fZ)
    {
        _vFlip = vec2( fX, fY );
        _fFlipZ = fZ;
    }
    void reset(const bool bAnimate);

};
