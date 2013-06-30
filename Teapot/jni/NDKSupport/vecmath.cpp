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
// vecmath.cpp
//--------------------------------------------------------------------------------

#include "vecmath.h"


//--------------------------------------------------------------------------------
// vec3
//--------------------------------------------------------------------------------
vec3::vec3(const vec4& vec)
{
    x = vec.x;  y = vec.y;  z = vec.z;
}

//--------------------------------------------------------------------------------
// vec4
//--------------------------------------------------------------------------------
vec4 vec4::operator*(const mat4& rhs) const
{
    vec4 out;
    out.x = x * rhs.f[0] + y * rhs.f[1] + z * rhs.f[2] + w * rhs.f[3];
    out.y = x * rhs.f[4] + y * rhs.f[5] + z * rhs.f[6] + w * rhs.f[7];
    out.z = x * rhs.f[8] + y * rhs.f[9] + z * rhs.f[10] + w * rhs.f[11];
    out.w = x * rhs.f[12] + y * rhs.f[13] + z * rhs.f[14] + w * rhs.f[15];
    return out;
}

//--------------------------------------------------------------------------------
// mat4
//--------------------------------------------------------------------------------
mat4::mat4()
{
    for(int32_t i = 0; i < 16; ++i)
        f[i] = 0.f;
}

mat4::mat4(const float* mIn )
{
    for(int32_t i = 0; i < 16; ++i)
        f[i] = mIn[i];
}

mat4 mat4::operator*(const mat4& rhs) const
{
        mat4 ret;
        ret.f[0] = f[0]*rhs.f[0] + f[4]*rhs.f[1] + f[8]*rhs.f[2] + f[12]*rhs.f[3];
        ret.f[1] = f[1]*rhs.f[0] + f[5]*rhs.f[1] + f[9]*rhs.f[2] + f[13]*rhs.f[3];
        ret.f[2] = f[2]*rhs.f[0] + f[6]*rhs.f[1] + f[10]*rhs.f[2] + f[14]*rhs.f[3];
        ret.f[3] = f[3]*rhs.f[0] + f[7]*rhs.f[1] + f[11]*rhs.f[2] + f[15]*rhs.f[3];

        ret.f[4] = f[0]*rhs.f[4] + f[4]*rhs.f[5] + f[8]*rhs.f[6] + f[12]*rhs.f[7];
        ret.f[5] = f[1]*rhs.f[4] + f[5]*rhs.f[5] + f[9]*rhs.f[6] + f[13]*rhs.f[7];
        ret.f[6] = f[2]*rhs.f[4] + f[6]*rhs.f[5] + f[10]*rhs.f[6] + f[14]*rhs.f[7];
        ret.f[7] = f[3]*rhs.f[4] + f[7]*rhs.f[5] + f[11]*rhs.f[6] + f[15]*rhs.f[7];

        ret.f[8] = f[0]*rhs.f[8] + f[4]*rhs.f[9] + f[8]*rhs.f[10] + f[12]*rhs.f[11];
        ret.f[9] = f[1]*rhs.f[8] + f[5]*rhs.f[9] + f[9]*rhs.f[10] + f[13]*rhs.f[11];
        ret.f[10] = f[2]*rhs.f[8] + f[6]*rhs.f[9] + f[10]*rhs.f[10] + f[14]*rhs.f[11];
        ret.f[11] = f[3]*rhs.f[8] + f[7]*rhs.f[9] + f[11]*rhs.f[10] + f[15]*rhs.f[11];

        ret.f[12] = f[0]*rhs.f[12] + f[4]*rhs.f[13] + f[8]*rhs.f[14] + f[12]*rhs.f[15];
        ret.f[13] = f[1]*rhs.f[12] + f[5]*rhs.f[13] + f[9]*rhs.f[14] + f[13]*rhs.f[15];
        ret.f[14] = f[2]*rhs.f[12] + f[6]*rhs.f[13] + f[10]*rhs.f[14] + f[14]*rhs.f[15];
        ret.f[15] = f[3]*rhs.f[12] + f[7]*rhs.f[13] + f[11]*rhs.f[14] + f[15]*rhs.f[15];

        return ret;
}

vec4 mat4::operator*(const vec4& rhs) const
{
    vec4 ret;
    ret.x = rhs.x*f[0] + rhs.y*f[4] + rhs.z*f[8] + rhs.w*f[12];
    ret.y = rhs.x*f[1] + rhs.y*f[5] + rhs.z*f[9] + rhs.w*f[13];
    ret.z = rhs.x*f[2] + rhs.y*f[6] + rhs.z*f[10] + rhs.w*f[14];
    ret.w = rhs.x*f[3] + rhs.y*f[7] + rhs.z*f[11] + rhs.w*f[15];
    return ret;
}

mat4 mat4::inverse()
{
    mat4 ret;
    float det_1;
    float pos = 0;
    float neg = 0;
    float temp;

    temp =  f[0] * f[5] * f[10];
    if (temp >= 0) pos += temp; else neg += temp;
    temp =   f[4] * f[9] * f[2];
    if (temp >= 0) pos += temp; else neg += temp;
    temp =   f[8] * f[1] * f[6];
    if (temp >= 0) pos += temp; else neg += temp;
    temp =   -f[8] * f[5] * f[2];
    if (temp >= 0) pos += temp; else neg += temp;
    temp =   -f[4] * f[1] * f[10];
    if (temp >= 0) pos += temp; else neg += temp;
    temp =   -f[0] * f[9] * f[6];
    if (temp >= 0) pos += temp; else neg += temp;
    det_1 = pos + neg;

    if (det_1 == 0.0)
    {
        //Error
    }
    else
    {
        det_1 = 1.0f / det_1;
        ret.f[0] =  ( f[ 5] * f[10] - f[ 9] * f[ 6] ) * det_1;
        ret.f[1] = -( f[ 1] * f[10] - f[ 9] * f[ 2] ) * det_1;
        ret.f[2] =  ( f[ 1] * f[ 6] - f[ 5] * f[ 2] ) * det_1;
        ret.f[4] = -( f[ 4] * f[10] - f[ 8] * f[ 6] ) * det_1;
        ret.f[5] =  ( f[ 0] * f[10] - f[ 8] * f[ 2] ) * det_1;
        ret.f[6] = -( f[ 0] * f[ 6] - f[ 4] * f[ 2] ) * det_1;
        ret.f[8] =  ( f[ 4] * f[ 9] - f[ 8] * f[ 5] ) * det_1;
        ret.f[9] = -( f[ 0] * f[ 9] - f[ 8] * f[ 1] ) * det_1;
        ret.f[10] = ( f[ 0] * f[ 5] - f[ 4] * f[ 1] ) * det_1;

        /* Calculate -C * inverse(A) */
        ret.f[12] = - ( f[12] *ret.f[0] + f[13] * ret.f[4] + f[14] *ret.f[8] );
        ret.f[13] = - ( f[12] * ret.f[1] + f[13] * ret.f[5] + f[14] * ret.f[9] );
        ret.f[14] = - ( f[12] * ret.f[2] + f[13] * ret.f[6] + f[14] * ret.f[10] );

        ret.f[ 3] = 0.0f;
        ret.f[ 7] = 0.0f;
        ret.f[11] = 0.0f;
        ret.f[15] = 1.0f;
    }

    *this = ret;
    return *this;
}

//--------------------------------------------------------------------------------
// Misc
//--------------------------------------------------------------------------------

mat4 mat4::rotationX(
    const float fAngle)
{
    mat4    ret;
    float   fCosine, fSine;

    fCosine = cosf(fAngle);
    fSine   = sinf(fAngle);

    ret.f[ 0]=1.0f; ret.f[ 4]=0.0f;     ret.f[ 8]=0.0f;     ret.f[12]=0.0f;
    ret.f[ 1]=0.0f; ret.f[ 5]=fCosine;  ret.f[ 9]=fSine;        ret.f[13]=0.0f;
    ret.f[ 2]=0.0f; ret.f[ 6]=-fSine;   ret.f[10]=fCosine;  ret.f[14]=0.0f;
    ret.f[ 3]=0.0f; ret.f[ 7]=0.0f;     ret.f[11]=0.0f;     ret.f[15]=1.0f;
    return ret;
}

mat4 mat4::rotationY(
    const float fAngle)
{
    mat4 ret;
    float       fCosine, fSine;

    fCosine = cosf(fAngle);
    fSine   = sinf(fAngle);

    ret.f[ 0]=fCosine;  ret.f[ 4]=0.0f; ret.f[ 8]=-fSine;   ret.f[12]=0.0f;
    ret.f[ 1]=0.0f;     ret.f[ 5]=1.0f; ret.f[ 9]=0.0f;     ret.f[13]=0.0f;
    ret.f[ 2]=fSine;        ret.f[ 6]=0.0f; ret.f[10]=fCosine;  ret.f[14]=0.0f;
    ret.f[ 3]=0.0f;     ret.f[ 7]=0.0f; ret.f[11]=0.0f;     ret.f[15]=1.0f;
    return ret;

}

mat4 mat4::rotationZ(
    const float fAngle)
{
    mat4    ret;
    float       fCosine, fSine;

    fCosine =   cosf(fAngle);
    fSine =     sinf(fAngle);

    ret.f[ 0]=fCosine;  ret.f[ 4]=fSine;    ret.f[ 8]=0.0f; ret.f[12]=0.0f;
    ret.f[ 1]=-fSine;   ret.f[ 5]=fCosine;  ret.f[ 9]=0.0f; ret.f[13]=0.0f;
    ret.f[ 2]=0.0f;     ret.f[ 6]=0.0f;     ret.f[10]=1.0f; ret.f[14]=0.0f;
    ret.f[ 3]=0.0f;     ret.f[ 7]=0.0f;     ret.f[11]=0.0f; ret.f[15]=1.0f;
    return ret;
}

mat4 mat4::translation(
    const float fX,
    const float fY,
    const float fZ)
{
    mat4    ret;
    ret.f[ 0]=1.0f; ret.f[ 4]=0.0f; ret.f[ 8]=0.0f; ret.f[12]=fX;
    ret.f[ 1]=0.0f; ret.f[ 5]=1.0f; ret.f[ 9]=0.0f; ret.f[13]=fY;
    ret.f[ 2]=0.0f; ret.f[ 6]=0.0f; ret.f[10]=1.0f; ret.f[14]=fZ;
    ret.f[ 3]=0.0f; ret.f[ 7]=0.0f; ret.f[11]=0.0f; ret.f[15]=1.0f;
    return ret;
}

mat4 mat4::translation(
    const vec3 vec)
{
    mat4    ret;
    ret.f[ 0]=1.0f; ret.f[ 4]=0.0f; ret.f[ 8]=0.0f; ret.f[12]=vec.x;
    ret.f[ 1]=0.0f; ret.f[ 5]=1.0f; ret.f[ 9]=0.0f; ret.f[13]=vec.y;
    ret.f[ 2]=0.0f; ret.f[ 6]=0.0f; ret.f[10]=1.0f; ret.f[14]=vec.z;
    ret.f[ 3]=0.0f; ret.f[ 7]=0.0f; ret.f[11]=0.0f; ret.f[15]=1.0f;
    return ret;
}

mat4 mat4::perspective(
    float width,
    float height,
    float nearPlane, float farPlane)
{
    float n2 = 2.0f * nearPlane;
    float rcpnmf = 1.f/(nearPlane - farPlane);

    mat4 result;
    result.f[0] = n2 / width;   result.f[4] = 0;    result.f[8] = 0;    result.f[12] = 0;
    result.f[1] = 0;    result.f[5] = n2 / height;  result.f[9] = 0;    result.f[13] = 0;
    result.f[2] = 0;    result.f[6] = 0;    result.f[10] = (farPlane+nearPlane)*rcpnmf; result.f[14] = farPlane*rcpnmf*n2;
    result.f[3] = 0;    result.f[7] = 0;    result.f[11] = -1.0;    result.f[15]=0;

    return result;
}

mat4 mat4::lookAt(const vec3& vEye, const vec3& vAt, const vec3& vUp)
{
    vec3 vForward, vUpNorm, vSide;
    mat4 result;

    vForward.x = vEye.x - vAt.x;
    vForward.y = vEye.y - vAt.y;
    vForward.z = vEye.z - vAt.z;

    vForward.normalize();
    vUpNorm = vUp;
    vUpNorm.normalize();
    vSide   = vUpNorm.cross( vForward);
    vUpNorm = vForward.cross(vSide);

    result.f[0]=vSide.x;    result.f[4]=vSide.y;    result.f[8]=vSide.z;        result.f[12]=0;
    result.f[1]=vUpNorm.x;  result.f[5]=vUpNorm.y;  result.f[9]=vUpNorm.z;      result.f[13]=0;
    result.f[2]=vForward.x; result.f[6]=vForward.y; result.f[10]=vForward.z;    result.f[14]=0;
    result.f[3]=0;          result.f[7]=0;          result.f[11]=0;             result.f[15]=1.0;

    result.postTranslate(-vEye.x, -vEye.y, -vEye.z);
    return result;
}

