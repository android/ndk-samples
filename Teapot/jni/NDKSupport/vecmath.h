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

#ifndef VECMATH_H_
#define VECMATH_H_

#include <math.h>
#include "JNIHelper.h"

class vec2;
class vec3;
class vec4;
class mat4;
//--------------------------------------------------------------------------------
// vec2
//--------------------------------------------------------------------------------
class vec2
{
    float x,y;
public:
    friend class vec3;
    friend class vec4;
    friend class mat4;
    friend class quaternion;

    vec2()
    {
        x = y = 0.f;
    }

    vec2(const float fX, const float fY)
    {
        x = fX; y = fY;
    }

    vec2(const vec2& vec)
    {
        x = vec.x;
        y = vec.y;
    }

    vec2(const float* pVec)
    {
        x = (*pVec++); y = (*pVec++);
    }

    //Operators
    vec2 operator*(const vec2& rhs) const
    {
        vec2 ret;
        ret.x = x*rhs.x;
        ret.y = y*rhs.y;
        return ret;
    }

    vec2 operator/(const vec2& rhs) const
    {
        vec2 ret;
        ret.x = x/rhs.x;
        ret.y = y/rhs.y;
        return ret;
    }

    vec2 operator+(const vec2& rhs) const
    {
        vec2 ret;
        ret.x = x + rhs.x;
        ret.y = y + rhs.y;
        return ret;
    }

    vec2 operator-(const vec2& rhs) const
    {
        vec2 ret;
        ret.x = x - rhs.x;
        ret.y = y - rhs.y;
        return ret;
    }

    vec2& operator+=(const vec2& rhs)
    {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }

    vec2& operator-=(const vec2& rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        return *this;
    }

    vec2& operator*=(const vec2& rhs)
    {
        x *= rhs.x;
        y *= rhs.y;
        return *this;
    }

    vec2& operator/=(const vec2& rhs)
    {
        x /= rhs.x;
        y /= rhs.y;
        return *this;
    }

    //External operators
    friend vec2 operator-(const vec2& rhs) { return vec2(rhs) *= -1; }

    friend vec2 operator*(const float lhs, const vec2&  rhs)
    {
        vec2 ret;
        ret.x = lhs * rhs.x;
        ret.y = lhs * rhs.y;
        return ret;
    }

    friend vec2 operator/(const float lhs, const vec2&  rhs)
    {
        vec2 ret;
        ret.x = lhs / rhs.x;
        ret.y = lhs / rhs.y;
        return ret;
    }

    //Operators with float
    vec2 operator*(const float& rhs) const
    {
        vec2 ret;
        ret.x = x * rhs;
        ret.y = y * rhs;
        return ret;
    }

    vec2& operator*=(const float& rhs)
    {
        x = x * rhs;
        y = y * rhs;
        return *this;
    }

    vec2 operator/(const float& rhs) const
    {
        vec2 ret;
        ret.x = x / rhs;
        ret.y = y / rhs;
        return ret;
    }

    vec2& operator/=(const float& rhs)
    {
        x = x / rhs;
        y = y / rhs;
        return *this;
    }

    //Compare
    bool operator==(const vec2& rhs) const
    {
        if(x != rhs.x || y != rhs.y ) return false;
        return true;
    }

    bool operator!=(const vec2& rhs) const
    {
        if(x == rhs.x)
            return false;

        return true;
    }

    float length() const
    {
        return sqrtf(x*x + y*y);
    }

    vec2 normalize()
    {
        float len = length();
        x = x / len;
        y = y / len;
        return *this;
    }

    float dot(const vec2& rhs)
    {
        return x * rhs.x + y*rhs.y;
    }

    bool validate()
    {
        if( isnan( x ) || isnan( y ) )
            return false;
        return true;
    }

    void value(float& fX, float& fY)
    {
        fX = x;
        fY = y;
    }

    void dump()
    {
        LOGI("vec2 %f %f", x, y);
    }
};

//--------------------------------------------------------------------------------
// vec3
//--------------------------------------------------------------------------------
class vec3
{
    float x,y,z;
public:
    friend class vec4;
    friend class mat4;
    friend class quaternion;

    vec3()
    {
        x = y = z = 0.f;
    }

    vec3(const float fX, const float fY, const float fZ)
    {
        x = fX; y = fY; z = fZ;
    }

    vec3(const vec3& vec)
    {
        x = vec.x;
        y = vec.y;
        z = vec.z;
    }

    vec3(const float* pVec)
    {
        x = (*pVec++); y = (*pVec++); z = *pVec;
    }

    vec3(const vec2& vec, float f )
    {
        x = vec.x;
        y = vec.y;
        z = f;
    }

    vec3(const vec4& vec );

    //Operators
    vec3 operator*(const vec3& rhs) const
    {
        vec3 ret;
        ret.x = x*rhs.x;
        ret.y = y*rhs.y;
        ret.z = z*rhs.z;
        return ret;
    }

    vec3 operator/(const vec3& rhs) const
    {
        vec3 ret;
        ret.x = x/rhs.x;
        ret.y = y/rhs.y;
        ret.z = z/rhs.z;
        return ret;
    }

    vec3 operator+(const vec3& rhs) const
    {
        vec3 ret;
        ret.x = x + rhs.x;
        ret.y = y + rhs.y;
        ret.z = z + rhs.z;
        return ret;
    }

    vec3 operator-(const vec3& rhs) const
    {
        vec3 ret;
        ret.x = x - rhs.x;
        ret.y = y - rhs.y;
        ret.z = z - rhs.z;
        return ret;
    }

    vec3& operator+=(const vec3& rhs)
    {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        return *this;
    }

    vec3& operator-=(const vec3& rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        return *this;
    }

    vec3& operator*=(const vec3& rhs)
    {
        x *= rhs.x;
        y *= rhs.y;
        z *= rhs.z;
        return *this;
    }

    vec3& operator/=(const vec3& rhs)
    {
        x /= rhs.x;
        y /= rhs.y;
        z /= rhs.z;
        return *this;
    }

    //External operators
    friend vec3 operator-(const vec3& rhs) { return vec3(rhs) *= -1; }

    friend vec3 operator*(const float lhs, const vec3&  rhs)
    {
        vec3 ret;
        ret.x = lhs * rhs.x;
        ret.y = lhs * rhs.y;
        ret.z = lhs * rhs.z;
        return ret;
    }

    friend vec3 operator/(const float lhs, const vec3&  rhs)
    {
        vec3 ret;
        ret.x = lhs / rhs.x;
        ret.y = lhs / rhs.y;
        ret.z = lhs / rhs.z;
        return ret;
    }

    //Operators with float
    vec3 operator*(const float& rhs) const
    {
        vec3 ret;
        ret.x = x * rhs;
        ret.y = y * rhs;
        ret.z = z * rhs;
        return ret;
    }

    vec3& operator*=(const float& rhs)
    {
        x = x * rhs;
        y = y * rhs;
        z = z * rhs;
        return *this;
    }

    vec3 operator/(const float& rhs) const
    {
        vec3 ret;
        ret.x = x / rhs;
        ret.y = y / rhs;
        ret.z = z / rhs;
        return ret;
    }

    vec3& operator/=(const float& rhs)
    {
        x = x / rhs;
        y = y / rhs;
        z = z / rhs;
        return *this;
    }

    //Compare
    bool operator==(const vec3& rhs) const
    {
        if(x != rhs.x || y != rhs.y || z != rhs.z) return false;
        return true;
    }

    bool operator!=(const vec3& rhs) const
    {
        if(x == rhs.x)
            return false;

        return true;
    }

    float length() const
    {
        return sqrtf(x*x + y*y + z*z);
    }

    vec3 normalize()
    {
        float len = length();
        x = x / len;
        y = y / len;
        z = z / len;
        return *this;
    }

    float dot(const vec3& rhs)
    {
        return x * rhs.x + y*rhs.y + z*rhs.z;
    }

    vec3 cross(const vec3& rhs)
    {
        vec3 ret;
        ret.x = y*rhs.z - z*rhs.y;
        ret.y = z*rhs.x - x*rhs.z;
        ret.z = x*rhs.y - y*rhs.x;
        return ret;
    }

    bool validate()
    {
        if( isnan( x ) || isnan( y ) || isnan( z ) )
            return false;
        return true;
    }

    void value(float& fX, float& fY, float& fZ)
    {
        fX = x;
        fY = y;
        fZ = z;
    }

    void dump()
    {
        LOGI("vec3 %f %f %f", x, y, z);
    }
};

//--------------------------------------------------------------------------------
// vec4
//--------------------------------------------------------------------------------
class vec4
{
    float x,y,z,w;
public:
    friend class vec3;
    friend class mat4;
    friend class quaternion;

    vec4()
    {
        x = y = z = w = 0.f;
    }

    vec4(const float fX, const float fY, const float fZ, const float fW )
    {
        x = fX; y = fY; z = fZ; w = fW;
    }

    vec4(const vec4& vec)
    {
        x = vec.x;
        y = vec.y;
        z = vec.z;
        w = vec.w;
    }

    vec4(const vec3& vec, const float fW )
    {
        x = vec.x;  y = vec.y;  z = vec.z; w = fW;
    }

    vec4(const float* pVec)
    {
        x = (*pVec++); y = (*pVec++); z = *pVec; w = *pVec;
    }

    //Operators
    vec4 operator*(const vec4& rhs) const
    {
        vec4 ret;
        ret.x = x*rhs.x;
        ret.y = y*rhs.y;
        ret.z = z*rhs.z;
        ret.w = z*rhs.w;
        return ret;
    }

    vec4 operator/(const vec4& rhs) const
    {
        vec4 ret;
        ret.x = x/rhs.x;
        ret.y = y/rhs.y;
        ret.z = z/rhs.z;
        ret.w = z/rhs.w;
        return ret;
    }

    vec4 operator+(const vec4& rhs) const
    {
        vec4 ret;
        ret.x = x+rhs.x;
        ret.y = y+rhs.y;
        ret.z = z+rhs.z;
        ret.w = z+rhs.w;
        return ret;
    }

    vec4 operator-(const vec4& rhs) const
    {
        vec4 ret;
        ret.x = x-rhs.x;
        ret.y = y-rhs.y;
        ret.z = z-rhs.z;
        ret.w = z-rhs.w;
        return ret;
    }

    vec4& operator+=(const vec4& rhs)
    {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        w += rhs.w;
        return *this;
    }

    vec4& operator-=(const vec4& rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        w -= rhs.w;
        return *this;
    }

    vec4& operator*=(const vec4& rhs)
    {
        x *= rhs.x;
        y *= rhs.y;
        z *= rhs.z;
        w *= rhs.w;
        return *this;
    }

    vec4& operator/=(const vec4& rhs)
    {
        x /= rhs.x;
        y /= rhs.y;
        z /= rhs.z;
        w /= rhs.w;
        return *this;
    }

    //External operators
    friend vec4 operator-(const vec4& rhs) { return vec4(rhs) *= -1; }

    friend vec4 operator*(const float lhs, const vec4&  rhs)
    {
        vec4 ret;
        ret.x = lhs * rhs.x;
        ret.y = lhs * rhs.y;
        ret.z = lhs * rhs.z;
        ret.w = lhs * rhs.w;
        return ret;
    }

    friend vec4 operator/(const float lhs, const vec4&  rhs)
    {
        vec4 ret;
        ret.x = lhs / rhs.x;
        ret.y = lhs / rhs.y;
        ret.z = lhs / rhs.z;
        ret.w = lhs / rhs.w;
        return ret;
    }

    //Operators with float
    vec4 operator*(const float& rhs) const
    {
        vec4 ret;
        ret.x = x * rhs;
        ret.y = y * rhs;
        ret.z = z * rhs;
        ret.w = w * rhs;
        return ret;
    }

    vec4& operator*=(const float& rhs)
    {
        x = x * rhs;
        y = y * rhs;
        z = z * rhs;
        w = w * rhs;
        return *this;
    }

    vec4 operator/(const float& rhs) const
    {
        vec4 ret;
        ret.x = x / rhs;
        ret.y = y / rhs;
        ret.z = z /rhs;
        ret.w = w /rhs;
        return ret;
    }

    vec4& operator/=(const float& rhs)
    {
        x = x / rhs;
        y = y / rhs;
        z = z / rhs;
        w = w / rhs;
        return *this;
    }

    //Compare
    bool operator==(const vec4& rhs) const
    {
        if(x != rhs.x || y != rhs.y || z != rhs.z || w != rhs.w) return false;
        return true;
    }

    bool operator!=(const vec4& rhs) const
    {
        if(x == rhs.x)
            return false;

        return true;
    }

    vec4 operator*(const mat4& rhs) const;

    float length() const
    {
        return sqrtf(x*x + y*y + z*z + w*w);
    }

    vec4 normalize()
    {
        float len = length();
        x = x / len;
        y = y / len;
        z = z / len;
        w = w / len;
        return *this;
    }

    float dot(const vec3& rhs)
    {
        return x * rhs.x + y*rhs.y + z*rhs.z;
    }

    vec3 cross(const vec3& rhs)
    {
        vec3 ret;
        ret.x = y*rhs.z - z*rhs.y;
        ret.y = z*rhs.x - x*rhs.z;
        ret.z = x*rhs.y - y*rhs.x;
        return ret;
    }

    bool validate()
    {
        if( isnan( x ) || isnan( y ) || isnan( z ) || isnan( w ))
            return false;
        return true;
    }

    void value(float& fX, float& fY, float& fZ, float& fW)
    {
        fX = x;
        fY = y;
        fZ = z;
        fW = w;
    }
};

//--------------------------------------------------------------------------------
// mat4
//--------------------------------------------------------------------------------
class mat4
{
    float f[16];
public:
    friend class vec3;
    friend class vec4;
    friend class quaternion;

    mat4();
    mat4(const float*);

    mat4 operator*(const mat4& rhs) const;
    vec4 operator*(const vec4& rhs) const;

    mat4 operator+(const mat4& rhs) const
    {
        mat4 ret;
        for(int32_t i=0; i < 16; ++i)
        {
            ret.f[i] = f[i] + rhs.f[i];
        }
        return ret;
    }

    mat4 operator-(const mat4& rhs) const
    {
        mat4 ret;
        for(int32_t i=0; i < 16; ++i)
        {
            ret.f[i] = f[i] - rhs.f[i];
        }
        return ret;
    }

    mat4& operator+=(const mat4& rhs)
    {
        for(int32_t i=0; i < 16; ++i)
        {
            f[i] += rhs.f[i];
        }
        return *this;
    }

    mat4& operator-=(const mat4& rhs)
    {
        for(int32_t i=0; i < 16; ++i)
        {
            f[i] -= rhs.f[i];
        }
        return *this;
    }

    mat4& operator*=(const mat4& rhs)
    {
        mat4 ret;
        ret.f[0] =  f[0] * rhs.f[0] + f[4] * rhs.f[1] + f[8] * rhs.f[2] + f[12] * rhs.f[3];
        ret.f[1] =  f[1] * rhs.f[0] + f[5] * rhs.f[1] + f[9] * rhs.f[2] + f[13] * rhs.f[3];
        ret.f[2] =  f[2] * rhs.f[0] + f[6] * rhs.f[1] + f[10] * rhs.f[2] + f[14] * rhs.f[3];
        ret.f[3] =  f[3] * rhs.f[0] + f[7] * rhs.f[1] + f[11] * rhs.f[2] + f[15] * rhs.f[3];

        ret.f[4] =  f[0] * rhs.f[4] + f[4] * rhs.f[5] + f[8] * rhs.f[6] + f[12] * rhs.f[7];
        ret.f[5] =  f[1] * rhs.f[4] + f[5] * rhs.f[5] + f[9] * rhs.f[6] + f[13] * rhs.f[7];
        ret.f[6] =  f[2] * rhs.f[4] + f[6] * rhs.f[5] + f[10] * rhs.f[6] + f[14] * rhs.f[7];
        ret.f[7] =  f[3] * rhs.f[4] + f[7] * rhs.f[5] + f[11] * rhs.f[6] + f[15] * rhs.f[7];

        ret.f[8] =  f[0] * rhs.f[8] + f[4] * rhs.f[9] + f[8] * rhs.f[10] + f[12] * rhs.f[11];
        ret.f[9] =  f[1] * rhs.f[8] + f[5] * rhs.f[9] + f[9] * rhs.f[10] + f[13] * rhs.f[11];
        ret.f[10] = f[2] * rhs.f[8] + f[6] * rhs.f[9] + f[10] * rhs.f[10] + f[14] * rhs.f[11];
        ret.f[11] = f[3] * rhs.f[8] + f[7] * rhs.f[9] + f[11] * rhs.f[10] + f[15] * rhs.f[11];

        ret.f[12] = f[0] * rhs.f[12] + f[4] * rhs.f[13] + f[8] * rhs.f[14] + f[12] * rhs.f[15];
        ret.f[13] = f[1] * rhs.f[12] + f[5] * rhs.f[13] + f[9] * rhs.f[14] + f[13] * rhs.f[15];
        ret.f[14] = f[2] * rhs.f[12] + f[6] * rhs.f[13] + f[10] * rhs.f[14] + f[14] * rhs.f[15];
        ret.f[15] = f[3] * rhs.f[12] + f[7] * rhs.f[13] + f[11] * rhs.f[14] + f[15] * rhs.f[15];

        *this = ret;
        return *this;
    }

    mat4 operator*(const float rhs)
    {
        mat4 ret;
        for(int32_t i=0; i < 16; ++i)
        {
            ret.f[i] = f[i] * rhs;
        }
        return ret;
    }

    mat4& operator*=(const float rhs)
    {
        for(int32_t i=0; i < 16; ++i)
        {
            f[i] *= rhs;
        }
        return *this;
    }

    mat4& operator=(const mat4& rhs)
    {
        for(int32_t i=0; i < 16; ++i)
        {
            f[i] = rhs.f[i];
        }
        return *this;
    }

    mat4 inverse();

    mat4 transpose()
    {
        mat4 ret;
        ret.f[0] = f[0];        ret.f[1] = f[4];        ret.f[2] = f[8];        ret.f[3] = f[12];
        ret.f[4] = f[1];        ret.f[5] = f[5];        ret.f[6] = f[9];        ret.f[7] = f[13];
        ret.f[8] = f[2];        ret.f[9] = f[6];        ret.f[10] = f[10];      ret.f[11] = f[14];
        ret.f[12] = f[3];       ret.f[13] = f[7];       ret.f[14] = f[11];      ret.f[15] = f[15];
        *this = ret;
        return *this;
    }

    mat4& postTranslate(float tx, float ty, float tz)
    {
        f[12] += (tx*f[0])+(ty*f[4])+(tz*f[8]);
        f[13] += (tx*f[1])+(ty*f[5])+(tz*f[9]);
        f[14] += (tx*f[2])+(ty*f[6])+(tz*f[10]);
        f[15] += (tx*f[3])+(ty*f[7])+(tz*f[11]);
        return *this;
    }

    float* ptr() {
        return f;
    }

    //--------------------------------------------------------------------------------
    // Misc
    //--------------------------------------------------------------------------------
    static mat4 perspective(
            float width,
            float height,
            float nearPlane, float farPlane);

    static mat4 lookAt(const vec3& vEye, const vec3& vAt, const vec3& vUp);

    static mat4 translation(
            const float fX,
            const float fY,
            const float fZ);
    static mat4 translation(
            const vec3 vec);

    static mat4 rotationX(
            const float fAngle);

    static mat4 rotationY(const float fAngle);

    static mat4 rotationZ(const float fAngle);

    static mat4 identity()
    {
        mat4 ret;
        ret.f[0] = 1.f; ret.f[1] = 0;   ret.f[2] = 0;   ret.f[3] = 0;
        ret.f[4] = 0;   ret.f[5] = 1.f; ret.f[6] = 0;   ret.f[7] = 0;
        ret.f[8] = 0;   ret.f[9] = 0;   ret.f[10] = 1.f;ret.f[11] = 0;
        ret.f[12] = 0;  ret.f[13] = 0;  ret.f[14] = 0;  ret.f[15] = 1.f;
        return ret;
    }

    void dump()
    {
        int32_t i = 0;
        LOGI("%f %f %f %f", f[0], f[1], f[2], f[3]);
        LOGI("%f %f %f %f", f[4], f[5], f[6], f[7]);
        LOGI("%f %f %f %f", f[8], f[9], f[10], f[11]);
        LOGI("%f %f %f %f", f[12], f[13], f[14], f[15]);
    }
};

//--------------------------------------------------------------------------------
// quaternion
//--------------------------------------------------------------------------------
class quaternion
{
    float x,y,z,w;
public:
    friend class vec3;
    friend class vec4;
    friend class mat4;

    quaternion()
    {
        x = 0.f;
        y = 0.f;
        z = 0.f;
        w = 1.f;
    }

    quaternion(const float fX, const float fY, const float fZ, const float fW)
    {
        x = fX;
        y = fY;
        z = fZ;
        w = fW;
    }

    quaternion(const vec3 vec, const float fW)
    {
        x = vec.x;
        y = vec.y;
        z = vec.z;
        w = fW;
    }

    quaternion(const float* p)
    {
        x = *p++;
        y = *p++;
        z = *p++;
        w = *p++;
    }

    quaternion operator*(const quaternion rhs)
    {
        quaternion ret;
        ret.x = x * rhs.w + y * rhs.z - z * rhs.y + w * rhs.x;
        ret.y = -x * rhs.z + y * rhs.w + z * rhs.x + w * rhs.y;
        ret.z = x * rhs.y - y * rhs.x + z * rhs.w + w * rhs.z;
        ret.w = -x * rhs.x - y * rhs.y - z * rhs.z + w * rhs.w;
        return ret;
    }

    quaternion& operator*=(const quaternion rhs)
    {
        quaternion ret;
        ret.x = x * rhs.w + y * rhs.z - z * rhs.y + w * rhs.x;
        ret.y = -x * rhs.z + y * rhs.w + z * rhs.x + w * rhs.y;
        ret.z = x * rhs.y - y * rhs.x + z * rhs.w + w * rhs.z;
        ret.w = -x * rhs.x - y * rhs.y - z * rhs.z + w * rhs.w;
        *this = ret;
        return *this;
    }

    quaternion conjugate()
    {
        x = -x;
        y = -y;
        z = -z;
        return *this;
    }

    //Non destuctive version
    quaternion conjugated()
    {
        quaternion ret;
        ret.x = -x;
        ret.y = -y;
        ret.z = -z;
        ret.w = w;
        return ret;
    }

    void toMatrix(mat4& mat)
    {
        float x2 = x * x * 2.0f;
        float y2 = y * y * 2.0f;
        float z2 = z * z * 2.0f;
        float xy = x * y * 2.0f;
        float yz = y * z * 2.0f;
        float zx = z * x * 2.0f;
        float xw = x * w * 2.0f;
        float yw = y * w * 2.0f;
        float zw = z * w * 2.0f;

        mat.f[0] = 1.0f - y2 - z2;
        mat.f[1] = xy + zw;
        mat.f[2] = zx - yw;
        mat.f[4] = xy - zw;
        mat.f[5] = 1.0f - z2 - x2;
        mat.f[6] = yz + xw;
        mat.f[8] = zx + yw;
        mat.f[9] = yz - xw;
        mat.f[10] = 1.0f - x2 - y2;

        mat.f[3] = mat.f[7] = mat.f[11] = mat.f[12] = mat.f[13] = mat.f[14] = 0.0f;
        mat.f[15] = 1.0f;
    }

    void toMatrixPreserveTranslate(mat4& mat)
    {
        float x2 = x * x * 2.0f;
        float y2 = y * y * 2.0f;
        float z2 = z * z * 2.0f;
        float xy = x * y * 2.0f;
        float yz = y * z * 2.0f;
        float zx = z * x * 2.0f;
        float xw = x * w * 2.0f;
        float yw = y * w * 2.0f;
        float zw = z * w * 2.0f;

        mat.f[0] = 1.0f - y2 - z2;
        mat.f[1] = xy + zw;
        mat.f[2] = zx - yw;
        mat.f[4] = xy - zw;
        mat.f[5] = 1.0f - z2 - x2;
        mat.f[6] = yz + xw;
        mat.f[8] = zx + yw;
        mat.f[9] = yz - xw;
        mat.f[10] = 1.0f - x2 - y2;

        mat.f[3] = mat.f[7] = mat.f[11] = 0.0f;
        mat.f[15] = 1.0f;
    }

    static quaternion rotationAxis(const vec3 axis, const float angle )
    {
        quaternion ret;
        float s = sinf(angle / 2);
        ret.x =  s * axis.x;
        ret.y =  s * axis.y;
        ret.z =  s * axis.z;
        ret.w = cosf( angle / 2 );
        return ret;
    }

    void value(float& fX, float& fY, float& fZ, float& fW)
    {
        fX = x;
        fY = y;
        fZ = z;
        fW = w;
    }
};

#endif /* VECMATH_H_ */
