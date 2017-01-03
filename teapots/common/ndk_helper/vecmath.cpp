/*
 * Copy_right 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * y_ou may_ not use this file ex_cept in compliance with the License.
 * You may_ obtain a copy_ of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by_ applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either ex_press or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//--------------------------------------------------------------------------------
// vecmath.cpp
//--------------------------------------------------------------------------------
#include "vecmath.h"

namespace ndk_helper {

//--------------------------------------------------------------------------------
// vec3
//--------------------------------------------------------------------------------
Vec3::Vec3(const Vec4& vec) {
  x_ = vec.x_;
  y_ = vec.y_;
  z_ = vec.z_;
}

//--------------------------------------------------------------------------------
// vec4
//--------------------------------------------------------------------------------
Vec4 Vec4::operator*(const Mat4& rhs) const {
  Vec4 out;
  out.x_ = x_ * rhs.f_[0] + y_ * rhs.f_[1] + z_ * rhs.f_[2] + w_ * rhs.f_[3];
  out.y_ = x_ * rhs.f_[4] + y_ * rhs.f_[5] + z_ * rhs.f_[6] + w_ * rhs.f_[7];
  out.z_ = x_ * rhs.f_[8] + y_ * rhs.f_[9] + z_ * rhs.f_[10] + w_ * rhs.f_[11];
  out.w_ =
      x_ * rhs.f_[12] + y_ * rhs.f_[13] + z_ * rhs.f_[14] + w_ * rhs.f_[15];
  return out;
}

//--------------------------------------------------------------------------------
// mat4
//--------------------------------------------------------------------------------
Mat4::Mat4() {
  for (int32_t i = 0; i < 16; ++i) f_[i] = 0.f;
  // column major identity matrix
  f_[0] = f_[5] = f_[10] = f_[15] = 1.0f;
}

Mat4::Mat4(const float* mIn) {
  for (int32_t i = 0; i < 16; ++i) f_[i] = mIn[i];
}

Mat4 Mat4::operator*(const Mat4& rhs) const {
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

  return ret;
}

Vec4 Mat4::operator*(const Vec4& rhs) const {
  Vec4 ret;
  ret.x_ = rhs.x_ * f_[0] + rhs.y_ * f_[4] + rhs.z_ * f_[8] + rhs.w_ * f_[12];
  ret.y_ = rhs.x_ * f_[1] + rhs.y_ * f_[5] + rhs.z_ * f_[9] + rhs.w_ * f_[13];
  ret.z_ = rhs.x_ * f_[2] + rhs.y_ * f_[6] + rhs.z_ * f_[10] + rhs.w_ * f_[14];
  ret.w_ = rhs.x_ * f_[3] + rhs.y_ * f_[7] + rhs.z_ * f_[11] + rhs.w_ * f_[15];
  return ret;
}

Mat4 Mat4::Inverse() {
  Mat4 ret;
  float det_1;
  float pos = 0;
  float neg = 0;
  float temp;

  temp = f_[0] * f_[5] * f_[10];
  if (temp >= 0)
    pos += temp;
  else
    neg += temp;
  temp = f_[4] * f_[9] * f_[2];
  if (temp >= 0)
    pos += temp;
  else
    neg += temp;
  temp = f_[8] * f_[1] * f_[6];
  if (temp >= 0)
    pos += temp;
  else
    neg += temp;
  temp = -f_[8] * f_[5] * f_[2];
  if (temp >= 0)
    pos += temp;
  else
    neg += temp;
  temp = -f_[4] * f_[1] * f_[10];
  if (temp >= 0)
    pos += temp;
  else
    neg += temp;
  temp = -f_[0] * f_[9] * f_[6];
  if (temp >= 0)
    pos += temp;
  else
    neg += temp;
  det_1 = pos + neg;

  if (det_1 == 0.0) {
    // Error
  } else {
    det_1 = 1.0f / det_1;
    ret.f_[0] = (f_[5] * f_[10] - f_[9] * f_[6]) * det_1;
    ret.f_[1] = -(f_[1] * f_[10] - f_[9] * f_[2]) * det_1;
    ret.f_[2] = (f_[1] * f_[6] - f_[5] * f_[2]) * det_1;
    ret.f_[4] = -(f_[4] * f_[10] - f_[8] * f_[6]) * det_1;
    ret.f_[5] = (f_[0] * f_[10] - f_[8] * f_[2]) * det_1;
    ret.f_[6] = -(f_[0] * f_[6] - f_[4] * f_[2]) * det_1;
    ret.f_[8] = (f_[4] * f_[9] - f_[8] * f_[5]) * det_1;
    ret.f_[9] = -(f_[0] * f_[9] - f_[8] * f_[1]) * det_1;
    ret.f_[10] = (f_[0] * f_[5] - f_[4] * f_[1]) * det_1;

    /* Calculate -C * inverse(A) */
    ret.f_[12] =
        -(f_[12] * ret.f_[0] + f_[13] * ret.f_[4] + f_[14] * ret.f_[8]);
    ret.f_[13] =
        -(f_[12] * ret.f_[1] + f_[13] * ret.f_[5] + f_[14] * ret.f_[9]);
    ret.f_[14] =
        -(f_[12] * ret.f_[2] + f_[13] * ret.f_[6] + f_[14] * ret.f_[10]);

    ret.f_[3] = 0.0f;
    ret.f_[7] = 0.0f;
    ret.f_[11] = 0.0f;
    ret.f_[15] = 1.0f;
  }

  *this = ret;
  return *this;
}

//--------------------------------------------------------------------------------
// Misc
//--------------------------------------------------------------------------------
Mat4 Mat4::RotationX(const float fAngle) {
  Mat4 ret;
  float fCosine, fSine;

  fCosine = cosf(fAngle);
  fSine = sinf(fAngle);

  ret.f_[0] = 1.0f;
  ret.f_[4] = 0.0f;
  ret.f_[8] = 0.0f;
  ret.f_[12] = 0.0f;
  ret.f_[1] = 0.0f;
  ret.f_[5] = fCosine;
  ret.f_[9] = fSine;
  ret.f_[13] = 0.0f;
  ret.f_[2] = 0.0f;
  ret.f_[6] = -fSine;
  ret.f_[10] = fCosine;
  ret.f_[14] = 0.0f;
  ret.f_[3] = 0.0f;
  ret.f_[7] = 0.0f;
  ret.f_[11] = 0.0f;
  ret.f_[15] = 1.0f;
  return ret;
}

Mat4 Mat4::RotationY(const float fAngle) {
  Mat4 ret;
  float fCosine, fSine;

  fCosine = cosf(fAngle);
  fSine = sinf(fAngle);

  ret.f_[0] = fCosine;
  ret.f_[4] = 0.0f;
  ret.f_[8] = -fSine;
  ret.f_[12] = 0.0f;
  ret.f_[1] = 0.0f;
  ret.f_[5] = 1.0f;
  ret.f_[9] = 0.0f;
  ret.f_[13] = 0.0f;
  ret.f_[2] = fSine;
  ret.f_[6] = 0.0f;
  ret.f_[10] = fCosine;
  ret.f_[14] = 0.0f;
  ret.f_[3] = 0.0f;
  ret.f_[7] = 0.0f;
  ret.f_[11] = 0.0f;
  ret.f_[15] = 1.0f;
  return ret;
}

Mat4 Mat4::RotationZ(const float fAngle) {
  Mat4 ret;
  float fCosine, fSine;

  fCosine = cosf(fAngle);
  fSine = sinf(fAngle);

  ret.f_[0] = fCosine;
  ret.f_[4] = fSine;
  ret.f_[8] = 0.0f;
  ret.f_[12] = 0.0f;
  ret.f_[1] = -fSine;
  ret.f_[5] = fCosine;
  ret.f_[9] = 0.0f;
  ret.f_[13] = 0.0f;
  ret.f_[2] = 0.0f;
  ret.f_[6] = 0.0f;
  ret.f_[10] = 1.0f;
  ret.f_[14] = 0.0f;
  ret.f_[3] = 0.0f;
  ret.f_[7] = 0.0f;
  ret.f_[11] = 0.0f;
  ret.f_[15] = 1.0f;
  return ret;
}

Mat4 Mat4::Scale(const float scaleX, const float scaleY, const float scaleZ) {
  Mat4 ret;
  ret.f_[0] = scaleX;
  ret.f_[5] = scaleY;
  ret.f_[10] = scaleZ;
  ret.f_[1] = ret.f_[2] = ret.f_[3] = ret.f_[4] = ret.f_[6] = ret.f_[7] =
      ret.f_[8] = ret.f_[9] = ret.f_[11] = ret.f_[12] = ret.f_[13] =
          ret.f_[14] = 0.f;
  ret.f_[15] = 1.0f;
  return ret;
}

Mat4 Mat4::Translation(const float fX, const float fY, const float fZ) {
  Mat4 ret;
  ret.f_[0] = 1.0f;
  ret.f_[4] = 0.0f;
  ret.f_[8] = 0.0f;
  ret.f_[12] = fX;
  ret.f_[1] = 0.0f;
  ret.f_[5] = 1.0f;
  ret.f_[9] = 0.0f;
  ret.f_[13] = fY;
  ret.f_[2] = 0.0f;
  ret.f_[6] = 0.0f;
  ret.f_[10] = 1.0f;
  ret.f_[14] = fZ;
  ret.f_[3] = 0.0f;
  ret.f_[7] = 0.0f;
  ret.f_[11] = 0.0f;
  ret.f_[15] = 1.0f;
  return ret;
}

Mat4 Mat4::Translation(const Vec3 vec) {
  Mat4 ret;
  ret.f_[0] = 1.0f;
  ret.f_[4] = 0.0f;
  ret.f_[8] = 0.0f;
  ret.f_[12] = vec.x_;
  ret.f_[1] = 0.0f;
  ret.f_[5] = 1.0f;
  ret.f_[9] = 0.0f;
  ret.f_[13] = vec.y_;
  ret.f_[2] = 0.0f;
  ret.f_[6] = 0.0f;
  ret.f_[10] = 1.0f;
  ret.f_[14] = vec.z_;
  ret.f_[3] = 0.0f;
  ret.f_[7] = 0.0f;
  ret.f_[11] = 0.0f;
  ret.f_[15] = 1.0f;
  return ret;
}

Mat4 Mat4::Perspective(float width, float height, float nearPlane,
                       float farPlane) {
  float n2 = 2.0f * nearPlane;
  float rcpnmf = 1.f / (nearPlane - farPlane);

  Mat4 result;
  result.f_[0] = n2 / width;
  result.f_[4] = 0;
  result.f_[8] = 0;
  result.f_[12] = 0;
  result.f_[1] = 0;
  result.f_[5] = n2 / height;
  result.f_[9] = 0;
  result.f_[13] = 0;
  result.f_[2] = 0;
  result.f_[6] = 0;
  result.f_[10] = (farPlane + nearPlane) * rcpnmf;
  result.f_[14] = farPlane * rcpnmf * n2;
  result.f_[3] = 0;
  result.f_[7] = 0;
  result.f_[11] = -1.0;
  result.f_[15] = 0;

  return result;
}

Mat4 Mat4::Ortho2D(float left, float top, float right, float bottom) {
  const float zNear = -1.0f;
  const float zFar = 1.0f;
  const float inv_z = 1.0f / (zFar - zNear);
  const float inv_y = 1.0f / (-top + bottom);
  const float inv_x = 1.0f / (right - left);

  Mat4 result;
  result.f_[0] = 2.0f * inv_x;
  result.f_[1] = 0.0f;
  result.f_[2] = 0.0f;
  result.f_[3] = 0.0f;

  result.f_[4] = 0.0f;
  result.f_[5] = 2.0 * inv_y;
  result.f_[6] = 0.0f;
  result.f_[7] = 0.0f;

  result.f_[8] = 0.0f;
  result.f_[9] = 0.0f;
  result.f_[10] = -2.0f * inv_z;
  result.f_[11] = 0.0f;

  result.f_[12] = -(right + left) * inv_x;
  result.f_[13] = (top + bottom) * inv_y;
  result.f_[14] = -(zFar + zNear) * inv_z;
  result.f_[15] = 1.0f;
  return result;
}

Mat4 Mat4::LookAt(const Vec3& vec_eye, const Vec3& vec_at, const Vec3& vec_up) {
  Vec3 vec_forward, vec_up_norm, vec_side;
  Mat4 result;

  vec_forward.x_ = vec_eye.x_ - vec_at.x_;
  vec_forward.y_ = vec_eye.y_ - vec_at.y_;
  vec_forward.z_ = vec_eye.z_ - vec_at.z_;

  vec_forward.Normalize();
  vec_up_norm = vec_up;
  vec_up_norm.Normalize();
  vec_side = vec_up_norm.Cross(vec_forward);
  vec_up_norm = vec_forward.Cross(vec_side);

  result.f_[0] = vec_side.x_;
  result.f_[4] = vec_side.y_;
  result.f_[8] = vec_side.z_;
  result.f_[12] = 0;
  result.f_[1] = vec_up_norm.x_;
  result.f_[5] = vec_up_norm.y_;
  result.f_[9] = vec_up_norm.z_;
  result.f_[13] = 0;
  result.f_[2] = vec_forward.x_;
  result.f_[6] = vec_forward.y_;
  result.f_[10] = vec_forward.z_;
  result.f_[14] = 0;
  result.f_[3] = 0;
  result.f_[7] = 0;
  result.f_[11] = 0;
  result.f_[15] = 1.0;

  result.PostTranslate(-vec_eye.x_, -vec_eye.y_, -vec_eye.z_);
  return result;
}

}  // namespace ndkHelper
