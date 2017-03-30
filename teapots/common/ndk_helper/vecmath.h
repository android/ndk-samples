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

#include <cmath>
#include "JNIHelper.h"

namespace ndk_helper {

/******************************************************************
 * Helper class for vector math operations
 * Currently all implementations are in pure C++.
 * Each class is an opaque class so caller does not have a direct access
 * to each element. This is for an ease of future optimization to use vector
 *operations.
 *
 */

class Vec2;
class Vec3;
class Vec4;
class Mat4;

/******************************************************************
 * 2 elements vector class
 *
 */
class Vec2 {
 private:
  float x_;
  float y_;

 public:
  friend class Vec3;
  friend class Vec4;
  friend class Mat4;
  friend class Quaternion;

  Vec2() { x_ = y_ = 0.f; }

  Vec2(const float fX, const float fY) {
    x_ = fX;
    y_ = fY;
  }

  Vec2(const Vec2& vec) {
    x_ = vec.x_;
    y_ = vec.y_;
  }

  Vec2(const float* pVec) {
    x_ = (*pVec++);
    y_ = (*pVec++);
  }

  // Operators
  Vec2 operator*(const Vec2& rhs) const {
    Vec2 ret;
    ret.x_ = x_ * rhs.x_;
    ret.y_ = y_ * rhs.y_;
    return ret;
  }

  Vec2 operator/(const Vec2& rhs) const {
    Vec2 ret;
    ret.x_ = x_ / rhs.x_;
    ret.y_ = y_ / rhs.y_;
    return ret;
  }

  Vec2 operator+(const Vec2& rhs) const {
    Vec2 ret;
    ret.x_ = x_ + rhs.x_;
    ret.y_ = y_ + rhs.y_;
    return ret;
  }

  Vec2 operator-(const Vec2& rhs) const {
    Vec2 ret;
    ret.x_ = x_ - rhs.x_;
    ret.y_ = y_ - rhs.y_;
    return ret;
  }

  Vec2& operator+=(const Vec2& rhs) {
    x_ += rhs.x_;
    y_ += rhs.y_;
    return *this;
  }

  Vec2& operator-=(const Vec2& rhs) {
    x_ -= rhs.x_;
    y_ -= rhs.y_;
    return *this;
  }

  Vec2& operator*=(const Vec2& rhs) {
    x_ *= rhs.x_;
    y_ *= rhs.y_;
    return *this;
  }

  Vec2& operator/=(const Vec2& rhs) {
    x_ /= rhs.x_;
    y_ /= rhs.y_;
    return *this;
  }

  // External operators
  friend Vec2 operator-(const Vec2& rhs) { return Vec2(rhs) *= -1; }

  friend Vec2 operator*(const float lhs, const Vec2& rhs) {
    Vec2 ret;
    ret.x_ = lhs * rhs.x_;
    ret.y_ = lhs * rhs.y_;
    return ret;
  }

  friend Vec2 operator/(const float lhs, const Vec2& rhs) {
    Vec2 ret;
    ret.x_ = lhs / rhs.x_;
    ret.y_ = lhs / rhs.y_;
    return ret;
  }

  // Operators with float
  Vec2 operator*(const float& rhs) const {
    Vec2 ret;
    ret.x_ = x_ * rhs;
    ret.y_ = y_ * rhs;
    return ret;
  }

  Vec2& operator*=(const float& rhs) {
    x_ = x_ * rhs;
    y_ = y_ * rhs;
    return *this;
  }

  Vec2 operator/(const float& rhs) const {
    Vec2 ret;
    ret.x_ = x_ / rhs;
    ret.y_ = y_ / rhs;
    return ret;
  }

  Vec2& operator/=(const float& rhs) {
    x_ = x_ / rhs;
    y_ = y_ / rhs;
    return *this;
  }

  // Compare
  bool operator==(const Vec2& rhs) const {
    if (x_ != rhs.x_ || y_ != rhs.y_) return false;
    return true;
  }

  bool operator!=(const Vec2& rhs) const {
    if (x_ == rhs.x_) return false;

    return true;
  }

  float Length() const { return sqrtf(x_ * x_ + y_ * y_); }

  Vec2 Normalize() {
    float len = Length();
    x_ = x_ / len;
    y_ = y_ / len;
    return *this;
  }

  float Dot(const Vec2& rhs) { return x_ * rhs.x_ + y_ * rhs.y_; }

  bool Validate() {
    if (std::isnan(x_) || std::isnan(y_)) return false;
    return true;
  }

  void Value(float& fX, float& fY) {
    fX = x_;
    fY = y_;
  }

  void Dump() { LOGI("Vec2 %f %f", x_, y_); }
};

/******************************************************************
 * 3 elements vector class
 *
 */
class Vec3 {
 private:
  float x_, y_, z_;

 public:
  friend class Vec4;
  friend class Mat4;
  friend class Quaternion;

  Vec3() { x_ = y_ = z_ = 0.f; }

  Vec3(const float fX, const float fY, const float fZ) {
    x_ = fX;
    y_ = fY;
    z_ = fZ;
  }

  Vec3(const Vec3& vec) {
    x_ = vec.x_;
    y_ = vec.y_;
    z_ = vec.z_;
  }

  Vec3(const float* pVec) {
    x_ = (*pVec++);
    y_ = (*pVec++);
    z_ = *pVec;
  }

  Vec3(const Vec2& vec, float f) {
    x_ = vec.x_;
    y_ = vec.y_;
    z_ = f;
  }

  Vec3(const Vec4& vec);

  // Operators
  Vec3 operator*(const Vec3& rhs) const {
    Vec3 ret;
    ret.x_ = x_ * rhs.x_;
    ret.y_ = y_ * rhs.y_;
    ret.z_ = z_ * rhs.z_;
    return ret;
  }

  Vec3 operator/(const Vec3& rhs) const {
    Vec3 ret;
    ret.x_ = x_ / rhs.x_;
    ret.y_ = y_ / rhs.y_;
    ret.z_ = z_ / rhs.z_;
    return ret;
  }

  Vec3 operator+(const Vec3& rhs) const {
    Vec3 ret;
    ret.x_ = x_ + rhs.x_;
    ret.y_ = y_ + rhs.y_;
    ret.z_ = z_ + rhs.z_;
    return ret;
  }

  Vec3 operator-(const Vec3& rhs) const {
    Vec3 ret;
    ret.x_ = x_ - rhs.x_;
    ret.y_ = y_ - rhs.y_;
    ret.z_ = z_ - rhs.z_;
    return ret;
  }

  Vec3& operator+=(const Vec3& rhs) {
    x_ += rhs.x_;
    y_ += rhs.y_;
    z_ += rhs.z_;
    return *this;
  }

  Vec3& operator-=(const Vec3& rhs) {
    x_ -= rhs.x_;
    y_ -= rhs.y_;
    z_ -= rhs.z_;
    return *this;
  }

  Vec3& operator*=(const Vec3& rhs) {
    x_ *= rhs.x_;
    y_ *= rhs.y_;
    z_ *= rhs.z_;
    return *this;
  }

  Vec3& operator/=(const Vec3& rhs) {
    x_ /= rhs.x_;
    y_ /= rhs.y_;
    z_ /= rhs.z_;
    return *this;
  }

  // External operators
  friend Vec3 operator-(const Vec3& rhs) { return Vec3(rhs) *= -1; }

  friend Vec3 operator*(const float lhs, const Vec3& rhs) {
    Vec3 ret;
    ret.x_ = lhs * rhs.x_;
    ret.y_ = lhs * rhs.y_;
    ret.z_ = lhs * rhs.z_;
    return ret;
  }

  friend Vec3 operator/(const float lhs, const Vec3& rhs) {
    Vec3 ret;
    ret.x_ = lhs / rhs.x_;
    ret.y_ = lhs / rhs.y_;
    ret.z_ = lhs / rhs.z_;
    return ret;
  }

  // Operators with float
  Vec3 operator*(const float& rhs) const {
    Vec3 ret;
    ret.x_ = x_ * rhs;
    ret.y_ = y_ * rhs;
    ret.z_ = z_ * rhs;
    return ret;
  }

  Vec3& operator*=(const float& rhs) {
    x_ = x_ * rhs;
    y_ = y_ * rhs;
    z_ = z_ * rhs;
    return *this;
  }

  Vec3 operator/(const float& rhs) const {
    Vec3 ret;
    ret.x_ = x_ / rhs;
    ret.y_ = y_ / rhs;
    ret.z_ = z_ / rhs;
    return ret;
  }

  Vec3& operator/=(const float& rhs) {
    x_ = x_ / rhs;
    y_ = y_ / rhs;
    z_ = z_ / rhs;
    return *this;
  }

  // Compare
  bool operator==(const Vec3& rhs) const {
    if (x_ != rhs.x_ || y_ != rhs.y_ || z_ != rhs.z_) return false;
    return true;
  }

  bool operator!=(const Vec3& rhs) const {
    if (x_ == rhs.x_) return false;

    return true;
  }

  float Length() const { return sqrtf(x_ * x_ + y_ * y_ + z_ * z_); }

  Vec3 Normalize() {
    float len = Length();
    x_ = x_ / len;
    y_ = y_ / len;
    z_ = z_ / len;
    return *this;
  }

  float Dot(const Vec3& rhs) { return x_ * rhs.x_ + y_ * rhs.y_ + z_ * rhs.z_; }

  Vec3 Cross(const Vec3& rhs) {
    Vec3 ret;
    ret.x_ = y_ * rhs.z_ - z_ * rhs.y_;
    ret.y_ = z_ * rhs.x_ - x_ * rhs.z_;
    ret.z_ = x_ * rhs.y_ - y_ * rhs.x_;
    return ret;
  }

  bool Validate() {
    if (std::isnan(x_) || std::isnan(y_) || std::isnan(z_)) return false;
    return true;
  }

  void Value(float& fX, float& fY, float& fZ) {
    fX = x_;
    fY = y_;
    fZ = z_;
  }

  void Dump() { LOGI("Vec3 %f %f %f", x_, y_, z_); }
};

/******************************************************************
 * 4 elements vector class
 *
 */
class Vec4 {
 private:
  float x_, y_, z_, w_;

 public:
  friend class Vec3;
  friend class Mat4;
  friend class Quaternion;

  Vec4() { x_ = y_ = z_ = w_ = 0.f; }

  Vec4(const float fX, const float fY, const float fZ, const float fW) {
    x_ = fX;
    y_ = fY;
    z_ = fZ;
    w_ = fW;
  }

  Vec4(const Vec4& vec) {
    x_ = vec.x_;
    y_ = vec.y_;
    z_ = vec.z_;
    w_ = vec.w_;
  }

  Vec4(const Vec3& vec, const float fW) {
    x_ = vec.x_;
    y_ = vec.y_;
    z_ = vec.z_;
    w_ = fW;
  }

  Vec4(const float* pVec) {
    x_ = (*pVec++);
    y_ = (*pVec++);
    z_ = *pVec;
    w_ = *pVec;
  }

  // Operators
  Vec4 operator*(const Vec4& rhs) const {
    Vec4 ret;
    ret.x_ = x_ * rhs.x_;
    ret.y_ = y_ * rhs.y_;
    ret.z_ = z_ * rhs.z_;
    ret.w_ = z_ * rhs.w_;
    return ret;
  }

  Vec4 operator/(const Vec4& rhs) const {
    Vec4 ret;
    ret.x_ = x_ / rhs.x_;
    ret.y_ = y_ / rhs.y_;
    ret.z_ = z_ / rhs.z_;
    ret.w_ = z_ / rhs.w_;
    return ret;
  }

  Vec4 operator+(const Vec4& rhs) const {
    Vec4 ret;
    ret.x_ = x_ + rhs.x_;
    ret.y_ = y_ + rhs.y_;
    ret.z_ = z_ + rhs.z_;
    ret.w_ = z_ + rhs.w_;
    return ret;
  }

  Vec4 operator-(const Vec4& rhs) const {
    Vec4 ret;
    ret.x_ = x_ - rhs.x_;
    ret.y_ = y_ - rhs.y_;
    ret.z_ = z_ - rhs.z_;
    ret.w_ = z_ - rhs.w_;
    return ret;
  }

  Vec4& operator+=(const Vec4& rhs) {
    x_ += rhs.x_;
    y_ += rhs.y_;
    z_ += rhs.z_;
    w_ += rhs.w_;
    return *this;
  }

  Vec4& operator-=(const Vec4& rhs) {
    x_ -= rhs.x_;
    y_ -= rhs.y_;
    z_ -= rhs.z_;
    w_ -= rhs.w_;
    return *this;
  }

  Vec4& operator*=(const Vec4& rhs) {
    x_ *= rhs.x_;
    y_ *= rhs.y_;
    z_ *= rhs.z_;
    w_ *= rhs.w_;
    return *this;
  }

  Vec4& operator/=(const Vec4& rhs) {
    x_ /= rhs.x_;
    y_ /= rhs.y_;
    z_ /= rhs.z_;
    w_ /= rhs.w_;
    return *this;
  }

  // External operators
  friend Vec4 operator-(const Vec4& rhs) { return Vec4(rhs) *= -1; }

  friend Vec4 operator*(const float lhs, const Vec4& rhs) {
    Vec4 ret;
    ret.x_ = lhs * rhs.x_;
    ret.y_ = lhs * rhs.y_;
    ret.z_ = lhs * rhs.z_;
    ret.w_ = lhs * rhs.w_;
    return ret;
  }

  friend Vec4 operator/(const float lhs, const Vec4& rhs) {
    Vec4 ret;
    ret.x_ = lhs / rhs.x_;
    ret.y_ = lhs / rhs.y_;
    ret.z_ = lhs / rhs.z_;
    ret.w_ = lhs / rhs.w_;
    return ret;
  }

  // Operators with float
  Vec4 operator*(const float& rhs) const {
    Vec4 ret;
    ret.x_ = x_ * rhs;
    ret.y_ = y_ * rhs;
    ret.z_ = z_ * rhs;
    ret.w_ = w_ * rhs;
    return ret;
  }

  Vec4& operator*=(const float& rhs) {
    x_ = x_ * rhs;
    y_ = y_ * rhs;
    z_ = z_ * rhs;
    w_ = w_ * rhs;
    return *this;
  }

  Vec4 operator/(const float& rhs) const {
    Vec4 ret;
    ret.x_ = x_ / rhs;
    ret.y_ = y_ / rhs;
    ret.z_ = z_ / rhs;
    ret.w_ = w_ / rhs;
    return ret;
  }

  Vec4& operator/=(const float& rhs) {
    x_ = x_ / rhs;
    y_ = y_ / rhs;
    z_ = z_ / rhs;
    w_ = w_ / rhs;
    return *this;
  }

  // Compare
  bool operator==(const Vec4& rhs) const {
    if (x_ != rhs.x_ || y_ != rhs.y_ || z_ != rhs.z_ || w_ != rhs.w_)
      return false;
    return true;
  }

  bool operator!=(const Vec4& rhs) const {
    if (x_ == rhs.x_) return false;

    return true;
  }

  Vec4 operator*(const Mat4& rhs) const;

  float Length() const { return sqrtf(x_ * x_ + y_ * y_ + z_ * z_ + w_ * w_); }

  Vec4 Normalize() {
    float len = Length();
    x_ = x_ / len;
    y_ = y_ / len;
    z_ = z_ / len;
    w_ = w_ / len;
    return *this;
  }

  float Dot(const Vec3& rhs) { return x_ * rhs.x_ + y_ * rhs.y_ + z_ * rhs.z_; }

  Vec3 Cross(const Vec3& rhs) {
    Vec3 ret;
    ret.x_ = y_ * rhs.z_ - z_ * rhs.y_;
    ret.y_ = z_ * rhs.x_ - x_ * rhs.z_;
    ret.z_ = x_ * rhs.y_ - y_ * rhs.x_;
    return ret;
  }

  bool Validate() {
    if (std::isnan(x_) || std::isnan(y_) ||
        std::isnan(z_) || std::isnan(w_))
      return false;

    return true;
  }

  void Value(float& fX, float& fY, float& fZ, float& fW) {
    fX = x_;
    fY = y_;
    fZ = z_;
    fW = w_;
  }
};

/******************************************************************
 * 4x4 matrix
 *
 */
class Mat4 {
 private:
  float f_[16];

 public:
  friend class Vec3;
  friend class Vec4;
  friend class Quaternion;

  Mat4();
  Mat4(const float*);

  Mat4 operator*(const Mat4& rhs) const;
  Vec4 operator*(const Vec4& rhs) const;

  Mat4 operator+(const Mat4& rhs) const {
    Mat4 ret;
    for (int32_t i = 0; i < 16; ++i) {
      ret.f_[i] = f_[i] + rhs.f_[i];
    }
    return ret;
  }

  Mat4 operator-(const Mat4& rhs) const {
    Mat4 ret;
    for (int32_t i = 0; i < 16; ++i) {
      ret.f_[i] = f_[i] - rhs.f_[i];
    }
    return ret;
  }

  Mat4& operator+=(const Mat4& rhs) {
    for (int32_t i = 0; i < 16; ++i) {
      f_[i] += rhs.f_[i];
    }
    return *this;
  }

  Mat4& operator-=(const Mat4& rhs) {
    for (int32_t i = 0; i < 16; ++i) {
      f_[i] -= rhs.f_[i];
    }
    return *this;
  }

  Mat4& operator*=(const Mat4& rhs) {
    Mat4 ret;
    ret.f_[0] = f_[0] * rhs.f_[0] + f_[4] * rhs.f_[1] + f_[8] * rhs.f_[2] +
                f_[12] * rhs.f_[3];
    ret.f_[1] = f_[1] * rhs.f_[0] + f_[5] * rhs.f_[1] + f_[9] * rhs.f_[2] +
                f_[13] * rhs.f_[3];
    ret.f_[2] = f_[2] * rhs.f_[0] + f_[6] * rhs.f_[1] + f_[10] * rhs.f_[2] +
                f_[14] * rhs.f_[3];
    ret.f_[3] = f_[3] * rhs.f_[0] + f_[7] * rhs.f_[1] + f_[11] * rhs.f_[2] +
                f_[15] * rhs.f_[3];

    ret.f_[4] = f_[0] * rhs.f_[4] + f_[4] * rhs.f_[5] + f_[8] * rhs.f_[6] +
                f_[12] * rhs.f_[7];
    ret.f_[5] = f_[1] * rhs.f_[4] + f_[5] * rhs.f_[5] + f_[9] * rhs.f_[6] +
                f_[13] * rhs.f_[7];
    ret.f_[6] = f_[2] * rhs.f_[4] + f_[6] * rhs.f_[5] + f_[10] * rhs.f_[6] +
                f_[14] * rhs.f_[7];
    ret.f_[7] = f_[3] * rhs.f_[4] + f_[7] * rhs.f_[5] + f_[11] * rhs.f_[6] +
                f_[15] * rhs.f_[7];

    ret.f_[8] = f_[0] * rhs.f_[8] + f_[4] * rhs.f_[9] + f_[8] * rhs.f_[10] +
                f_[12] * rhs.f_[11];
    ret.f_[9] = f_[1] * rhs.f_[8] + f_[5] * rhs.f_[9] + f_[9] * rhs.f_[10] +
                f_[13] * rhs.f_[11];
    ret.f_[10] = f_[2] * rhs.f_[8] + f_[6] * rhs.f_[9] + f_[10] * rhs.f_[10] +
                 f_[14] * rhs.f_[11];
    ret.f_[11] = f_[3] * rhs.f_[8] + f_[7] * rhs.f_[9] + f_[11] * rhs.f_[10] +
                 f_[15] * rhs.f_[11];

    ret.f_[12] = f_[0] * rhs.f_[12] + f_[4] * rhs.f_[13] + f_[8] * rhs.f_[14] +
                 f_[12] * rhs.f_[15];
    ret.f_[13] = f_[1] * rhs.f_[12] + f_[5] * rhs.f_[13] + f_[9] * rhs.f_[14] +
                 f_[13] * rhs.f_[15];
    ret.f_[14] = f_[2] * rhs.f_[12] + f_[6] * rhs.f_[13] + f_[10] * rhs.f_[14] +
                 f_[14] * rhs.f_[15];
    ret.f_[15] = f_[3] * rhs.f_[12] + f_[7] * rhs.f_[13] + f_[11] * rhs.f_[14] +
                 f_[15] * rhs.f_[15];

    *this = ret;
    return *this;
  }

  Mat4 operator*(const float rhs) {
    Mat4 ret;
    for (int32_t i = 0; i < 16; ++i) {
      ret.f_[i] = f_[i] * rhs;
    }
    return ret;
  }

  Mat4& operator*=(const float rhs) {
    for (int32_t i = 0; i < 16; ++i) {
      f_[i] *= rhs;
    }
    return *this;
  }

  Mat4& operator=(const Mat4& rhs) {
    for (int32_t i = 0; i < 16; ++i) {
      f_[i] = rhs.f_[i];
    }
    return *this;
  }

  Mat4 Inverse();

  Mat4 Transpose() {
    Mat4 ret;
    ret.f_[0] = f_[0];
    ret.f_[1] = f_[4];
    ret.f_[2] = f_[8];
    ret.f_[3] = f_[12];
    ret.f_[4] = f_[1];
    ret.f_[5] = f_[5];
    ret.f_[6] = f_[9];
    ret.f_[7] = f_[13];
    ret.f_[8] = f_[2];
    ret.f_[9] = f_[6];
    ret.f_[10] = f_[10];
    ret.f_[11] = f_[14];
    ret.f_[12] = f_[3];
    ret.f_[13] = f_[7];
    ret.f_[14] = f_[11];
    ret.f_[15] = f_[15];
    *this = ret;
    return *this;
  }

  Mat4& PostTranslate(float tx, float ty, float tz) {
    f_[12] += (tx * f_[0]) + (ty * f_[4]) + (tz * f_[8]);
    f_[13] += (tx * f_[1]) + (ty * f_[5]) + (tz * f_[9]);
    f_[14] += (tx * f_[2]) + (ty * f_[6]) + (tz * f_[10]);
    f_[15] += (tx * f_[3]) + (ty * f_[7]) + (tz * f_[11]);
    return *this;
  }

  float* Ptr() { return f_; }

  //--------------------------------------------------------------------------------
  // Misc
  //--------------------------------------------------------------------------------
  static Mat4 Perspective(float width, float height, float nearPlane,
                          float farPlane);
  static Mat4 Ortho2D(float left, float top, float right, float bottom);

  static Mat4 LookAt(const Vec3& vEye, const Vec3& vAt, const Vec3& vUp);

  static Mat4 Translation(const float fX, const float fY, const float fZ);
  static Mat4 Translation(const Vec3 vec);

  static Mat4 RotationX(const float angle);

  static Mat4 RotationY(const float angle);

  static Mat4 RotationZ(const float angle);

  static Mat4 Scale(const float scaleX, const float scaleY, const float scaleZ);

  static Mat4 Identity() {
    Mat4 ret;
    ret.f_[0] = 1.f;
    ret.f_[1] = 0;
    ret.f_[2] = 0;
    ret.f_[3] = 0;
    ret.f_[4] = 0;
    ret.f_[5] = 1.f;
    ret.f_[6] = 0;
    ret.f_[7] = 0;
    ret.f_[8] = 0;
    ret.f_[9] = 0;
    ret.f_[10] = 1.f;
    ret.f_[11] = 0;
    ret.f_[12] = 0;
    ret.f_[13] = 0;
    ret.f_[14] = 0;
    ret.f_[15] = 1.f;
    return ret;
  }

  void Dump() {
    LOGI("%f %f %f %f", f_[0], f_[1], f_[2], f_[3]);
    LOGI("%f %f %f %f", f_[4], f_[5], f_[6], f_[7]);
    LOGI("%f %f %f %f", f_[8], f_[9], f_[10], f_[11]);
    LOGI("%f %f %f %f", f_[12], f_[13], f_[14], f_[15]);
  }
};

/******************************************************************
 * Quaternion class
 *
 */
class Quaternion {
 private:
  float x_, y_, z_, w_;

 public:
  friend class Vec3;
  friend class Vec4;
  friend class Mat4;

  Quaternion() {
    x_ = 0.f;
    y_ = 0.f;
    z_ = 0.f;
    w_ = 1.f;
  }

  Quaternion(const float fX, const float fY, const float fZ, const float fW) {
    x_ = fX;
    y_ = fY;
    z_ = fZ;
    w_ = fW;
  }

  Quaternion(const Vec3 vec, const float fW) {
    x_ = vec.x_;
    y_ = vec.y_;
    z_ = vec.z_;
    w_ = fW;
  }

  Quaternion(const float* p) {
    x_ = *p++;
    y_ = *p++;
    z_ = *p++;
    w_ = *p++;
  }

  Quaternion operator*(const Quaternion rhs) {
    Quaternion ret;
    ret.x_ = x_ * rhs.w_ + y_ * rhs.z_ - z_ * rhs.y_ + w_ * rhs.x_;
    ret.y_ = -x_ * rhs.z_ + y_ * rhs.w_ + z_ * rhs.x_ + w_ * rhs.y_;
    ret.z_ = x_ * rhs.y_ - y_ * rhs.x_ + z_ * rhs.w_ + w_ * rhs.z_;
    ret.w_ = -x_ * rhs.x_ - y_ * rhs.y_ - z_ * rhs.z_ + w_ * rhs.w_;
    return ret;
  }

  Quaternion& operator*=(const Quaternion rhs) {
    Quaternion ret;
    ret.x_ = x_ * rhs.w_ + y_ * rhs.z_ - z_ * rhs.y_ + w_ * rhs.x_;
    ret.y_ = -x_ * rhs.z_ + y_ * rhs.w_ + z_ * rhs.x_ + w_ * rhs.y_;
    ret.z_ = x_ * rhs.y_ - y_ * rhs.x_ + z_ * rhs.w_ + w_ * rhs.z_;
    ret.w_ = -x_ * rhs.x_ - y_ * rhs.y_ - z_ * rhs.z_ + w_ * rhs.w_;
    *this = ret;
    return *this;
  }

  Quaternion Conjugate() {
    x_ = -x_;
    y_ = -y_;
    z_ = -z_;
    return *this;
  }

  // Non destuctive version
  Quaternion Conjugated() {
    Quaternion ret;
    ret.x_ = -x_;
    ret.y_ = -y_;
    ret.z_ = -z_;
    ret.w_ = w_;
    return ret;
  }

  void ToMatrix(Mat4& mat) {
    float x2 = x_ * x_ * 2.0f;
    float y2 = y_ * y_ * 2.0f;
    float z2 = z_ * z_ * 2.0f;
    float xy = x_ * y_ * 2.0f;
    float yz = y_ * z_ * 2.0f;
    float zx = z_ * x_ * 2.0f;
    float xw = x_ * w_ * 2.0f;
    float yw = y_ * w_ * 2.0f;
    float zw = z_ * w_ * 2.0f;

    mat.f_[0] = 1.0f - y2 - z2;
    mat.f_[1] = xy + zw;
    mat.f_[2] = zx - yw;
    mat.f_[4] = xy - zw;
    mat.f_[5] = 1.0f - z2 - x2;
    mat.f_[6] = yz + xw;
    mat.f_[8] = zx + yw;
    mat.f_[9] = yz - xw;
    mat.f_[10] = 1.0f - x2 - y2;

    mat.f_[3] = mat.f_[7] = mat.f_[11] = mat.f_[12] = mat.f_[13] = mat.f_[14] =
        0.0f;
    mat.f_[15] = 1.0f;
  }

  void ToMatrixPreserveTranslate(Mat4& mat) {
    float x2 = x_ * x_ * 2.0f;
    float y2 = y_ * y_ * 2.0f;
    float z2 = z_ * z_ * 2.0f;
    float xy = x_ * y_ * 2.0f;
    float yz = y_ * z_ * 2.0f;
    float zx = z_ * x_ * 2.0f;
    float xw = x_ * w_ * 2.0f;
    float yw = y_ * w_ * 2.0f;
    float zw = z_ * w_ * 2.0f;

    mat.f_[0] = 1.0f - y2 - z2;
    mat.f_[1] = xy + zw;
    mat.f_[2] = zx - yw;
    mat.f_[4] = xy - zw;
    mat.f_[5] = 1.0f - z2 - x2;
    mat.f_[6] = yz + xw;
    mat.f_[8] = zx + yw;
    mat.f_[9] = yz - xw;
    mat.f_[10] = 1.0f - x2 - y2;

    mat.f_[3] = mat.f_[7] = mat.f_[11] = 0.0f;
    mat.f_[15] = 1.0f;
  }

  static Quaternion RotationAxis(const Vec3 axis, const float angle) {
    Quaternion ret;
    float s = sinf(angle / 2);
    ret.x_ = s * axis.x_;
    ret.y_ = s * axis.y_;
    ret.z_ = s * axis.z_;
    ret.w_ = cosf(angle / 2);
    return ret;
  }

  void Value(float& fX, float& fY, float& fZ, float& fW) {
    fX = x_;
    fY = y_;
    fZ = z_;
    fW = w_;
  }
};

}  // namespace ndk_helper
#endif /* VECMATH_H_ */
