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

namespace ndk_helper {

const float TRANSFORM_FACTOR = 15.f;
const float TRANSFORM_FACTORZ = 10.f;

const float MOMENTUM_FACTOR_DECREASE = 0.85f;
const float MOMENTUM_FACTOR_DECREASE_SHIFT = 0.9f;
const float MOMENTUM_FACTOR = 0.8f;
const float MOMENTUM_FACTOR_THRESHOLD = 0.001f;

//----------------------------------------------------------
//  Ctor
//----------------------------------------------------------
TapCamera::TapCamera()
    : ball_radius_(0.75f),
      dragging_(false),
      pinching_(false),
      pinch_start_distance_SQ_(0.f),
      camera_rotation_(0.f),
      camera_rotation_start_(0.f),
      camera_rotation_now_(0.f),
      momentum_(false),
      momemtum_steps_(0.f),
      flip_z_(0.f) {
  // Init offset
  InitParameters();

  vec_flip_ = Vec2(1.f, -1.f);
  flip_z_ = -1.f;
  vec_pinch_transform_factor_ = Vec3(1.f, 1.f, 1.f);

  vec_ball_center_ = Vec2(0, 0);
  vec_ball_now_ = Vec2(0, 0);
  vec_ball_down_ = Vec2(0, 0);

  vec_pinch_start_ = Vec2(0, 0);
  vec_pinch_start_center_ = Vec2(0, 0);

  vec_flip_ = Vec2(0, 0);
}

void TapCamera::InitParameters() {
  // Init parameters
  vec_offset_ = Vec3();
  vec_offset_now_ = Vec3();

  quat_ball_rot_ = Quaternion();
  quat_ball_now_ = Quaternion();
  quat_ball_now_.ToMatrix(mat_rotation_);
  camera_rotation_ = 0.f;

  vec_drag_delta_ = Vec2();
  vec_offset_delta_ = Vec3();

  momentum_ = false;
}

//----------------------------------------------------------
//  Dtor
//----------------------------------------------------------
TapCamera::~TapCamera() {}

void TapCamera::Update() {
  if (momentum_) {
    float momenttum_steps = momemtum_steps_;

    // Momentum rotation
    Vec2 v = vec_drag_delta_;
    BeginDrag(Vec2());  // NOTE:This call reset _VDragDelta
    Drag(v * vec_flip_);

    // Momentum shift
    vec_offset_ += vec_offset_delta_;

    BallUpdate();
    EndDrag();

    // Decrease deltas
    vec_drag_delta_ = v * MOMENTUM_FACTOR_DECREASE;
    vec_offset_delta_ = vec_offset_delta_ * MOMENTUM_FACTOR_DECREASE_SHIFT;

    // Count steps
    momemtum_steps_ = momenttum_steps * MOMENTUM_FACTOR_DECREASE;
    if (momemtum_steps_ < MOMENTUM_FACTOR_THRESHOLD) {
      momentum_ = false;
    }
  } else {
    vec_drag_delta_ *= MOMENTUM_FACTOR;
    vec_offset_delta_ = vec_offset_delta_ * MOMENTUM_FACTOR;
    BallUpdate();
  }

  Vec3 vec = vec_offset_ + vec_offset_now_;
  Vec3 vec_tmp(TRANSFORM_FACTOR, -TRANSFORM_FACTOR, TRANSFORM_FACTORZ);

  vec *= vec_tmp * vec_pinch_transform_factor_;

  mat_transform_ = Mat4::Translation(vec);
}

void TapCamera::Update(const double time) {
  if (momentum_) {
    const float MOMENTAM_UNIT = 0.0166f;
    // Activate every 16.6msec
    if (time - time_stamp_ >= MOMENTAM_UNIT) {
      float momenttum_steps = momemtum_steps_;

      // Momentum rotation
      Vec2 v = vec_drag_delta_;
      BeginDrag(Vec2());  // NOTE:This call reset _VDragDelta
      Drag(v * vec_flip_);

      // Momentum shift
      vec_offset_ += vec_offset_delta_;

      BallUpdate();
      EndDrag();

      // Decrease deltas
      vec_drag_delta_ = v * MOMENTUM_FACTOR_DECREASE;
      vec_offset_delta_ = vec_offset_delta_ * MOMENTUM_FACTOR_DECREASE_SHIFT;

      // Count steps
      momemtum_steps_ = momenttum_steps * MOMENTUM_FACTOR_DECREASE;
      if (momemtum_steps_ < MOMENTUM_FACTOR_THRESHOLD) {
        momentum_ = false;
      }
      time_stamp_ = time;
    }
  } else {
    vec_drag_delta_ *= MOMENTUM_FACTOR;
    vec_offset_delta_ = vec_offset_delta_ * MOMENTUM_FACTOR;
    BallUpdate();
    time_stamp_ = time;
  }

  Vec3 vec = vec_offset_ + vec_offset_now_;
  Vec3 vec_tmp(TRANSFORM_FACTOR, -TRANSFORM_FACTOR, TRANSFORM_FACTORZ);

  vec *= vec_tmp * vec_pinch_transform_factor_;

  mat_transform_ = Mat4::Translation(vec);
}
Mat4& TapCamera::GetRotationMatrix() { return mat_rotation_; }

Mat4& TapCamera::GetTransformMatrix() { return mat_transform_; }

void TapCamera::Reset(const bool bAnimate) {
  InitParameters();
  Update();
}

//----------------------------------------------------------
// Drag control
//----------------------------------------------------------
void TapCamera::BeginDrag(const Vec2& v) {
  if (dragging_) EndDrag();

  if (pinching_) EndPinch();

  Vec2 vec = v * vec_flip_;
  vec_ball_now_ = vec;
  vec_ball_down_ = vec_ball_now_;

  dragging_ = true;
  momentum_ = false;
  vec_last_input_ = vec;
  vec_drag_delta_ = Vec2();
}

void TapCamera::EndDrag() {
  quat_ball_down_ = quat_ball_now_;
  quat_ball_rot_ = Quaternion();

  dragging_ = false;
  momentum_ = true;
  momemtum_steps_ = 1.0f;
}

void TapCamera::Drag(const Vec2& v) {
  if (!dragging_) return;

  Vec2 vec = v * vec_flip_;
  vec_ball_now_ = vec;

  vec_drag_delta_ = vec_drag_delta_ * MOMENTUM_FACTOR + (vec - vec_last_input_);
  vec_last_input_ = vec;
}

//----------------------------------------------------------
// Pinch controll
//----------------------------------------------------------
void TapCamera::BeginPinch(const Vec2& v1, const Vec2& v2) {
  if (dragging_) EndDrag();

  if (pinching_) EndPinch();

  BeginDrag(Vec2());

  vec_pinch_start_center_ = (v1 + v2) / 2.f;

  Vec2 vec = v1 - v2;
  float x_diff;
  float y_diff;
  vec.Value(x_diff, y_diff);

  pinch_start_distance_SQ_ = x_diff * x_diff + y_diff * y_diff;
  camera_rotation_start_ = atan2f(y_diff, x_diff);
  camera_rotation_now_ = 0;

  pinching_ = true;
  momentum_ = false;

  // Init momentum factors
  vec_offset_delta_ = Vec3();
}

void TapCamera::EndPinch() {
  pinching_ = false;
  momentum_ = true;
  momemtum_steps_ = 1.f;
  vec_offset_ += vec_offset_now_;
  camera_rotation_ += camera_rotation_now_;
  vec_offset_now_ = Vec3();

  camera_rotation_now_ = 0;

  EndDrag();
}

void TapCamera::Pinch(const Vec2& v1, const Vec2& v2) {
  if (!pinching_) return;

  // Update momentum factor
  vec_offset_last_ = vec_offset_now_;

  float x_diff, y_diff;
  Vec2 vec = v1 - v2;
  vec.Value(x_diff, y_diff);

  float fDistanceSQ = x_diff * x_diff + y_diff * y_diff;

  float f = pinch_start_distance_SQ_ / fDistanceSQ;
  if (f < 1.f)
    f = -1.f / f + 1.0f;
  else
    f = f - 1.f;
  if (std::isnan(f)) f = 0.f;

  vec = (v1 + v2) / 2.f - vec_pinch_start_center_;
  vec_offset_now_ = Vec3(vec, flip_z_ * f);

  // Update momentum factor
  vec_offset_delta_ = vec_offset_delta_ * MOMENTUM_FACTOR +
                      (vec_offset_now_ - vec_offset_last_);

  //
  // Update ration quaternion
  float fRotation = atan2f(y_diff, x_diff);
  camera_rotation_now_ = fRotation - camera_rotation_start_;

  // Trackball rotation
  quat_ball_rot_ = Quaternion(0.f, 0.f, sinf(-camera_rotation_now_ * 0.5f),
                              cosf(-camera_rotation_now_ * 0.5f));
}

//----------------------------------------------------------
// Trackball controll
//----------------------------------------------------------
void TapCamera::BallUpdate() {
  if (dragging_) {
    Vec3 vec_from = PointOnSphere(vec_ball_down_);
    Vec3 vec_to = PointOnSphere(vec_ball_now_);

    Vec3 vec = vec_from.Cross(vec_to);
    float w = vec_from.Dot(vec_to);

    Quaternion qDrag = Quaternion(vec, w);
    qDrag = qDrag * quat_ball_down_;
    quat_ball_now_ = quat_ball_rot_ * qDrag;
  }
  quat_ball_now_.ToMatrix(mat_rotation_);
}

Vec3 TapCamera::PointOnSphere(Vec2& point) {
  Vec3 ball_mouse;
  float mag;
  Vec2 vec = (point - vec_ball_center_) / ball_radius_;
  mag = vec.Dot(vec);
  if (mag > 1.f) {
    float scale = 1.f / sqrtf(mag);
    vec *= scale;
    ball_mouse = Vec3(vec, 0.f);
  } else {
    ball_mouse = Vec3(vec, sqrtf(1.f - mag));
  }
  return ball_mouse;
}

}  // namespace ndkHelper
