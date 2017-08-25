/*
 * Copyright (C) 2017 The Android Open Source Project
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
 *
 */
#include <android/input.h>
#include "mathfu/matrix.h"
#include "mathfu/glsl_mappings.h"
#include "ImageViewEngine.h"

const uint64_t kSwipeThreshold = static_cast<uint64_t>(1000000000);
const uint32_t kMinDistance = 100;
const uint32_t kMaxTapDistance = 10;
const uint32_t kMaxTapTime   = static_cast<uint64_t>(125000000);

void ImageViewEngine::ResetUserEventCache(void) {
  touchStartPos_ = mathfu::vec2(0.0f, 0.0f);
  startTime_ = static_cast<uint64_t>(0);
}

void ImageViewEngine::ProcessTapEvent(int x, int y) {
  // Get current display_ size:  the phone is in landscape mode ( in AndroidManifest.xml )
  int halfWidth = renderTargetWidth_ / 2;

  if (x > halfWidth - 10  && x < halfWidth + 10)
    return;

  uint32_t  mask = 1 << (x / halfWidth);

  renderModeBits_ = (renderModeBits_ & mask) ?
                    (renderModeBits_ & ~mask) :
                    (renderModeBits_ | mask);
  UpdateUI();
}

bool ImageViewEngine::ProcessInputEvent(const AInputEvent* event) {
  if(AMotionEvent_getPointerCount(event) > 1) {
    ResetUserEventCache();
    LOGE("more than one pointer action");
    return true;
  }

  int32_t action = AMotionEvent_getAction(event);
  if (action == AMOTION_EVENT_ACTION_CANCEL) {
    ResetUserEventCache();
    return true;
  }

  if(action == AMOTION_EVENT_ACTION_DOWN) {
    touchStartPos_.x = AMotionEvent_getX(event, 0);
    touchStartPos_.y = AMotionEvent_getY(event, 0);
    startTime_ = AMotionEvent_getEventTime(event);
    return true;
  } else if(action == AMOTION_EVENT_ACTION_UP) {
    uint64_t endTime = AMotionEvent_getEventTime(event);
    if (endTime - startTime_ > kSwipeThreshold) {
      return true;
    }

    mathfu::vec2 v2;
    v2.x = AMotionEvent_getX(event, 0);
    v2.y = AMotionEvent_getY(event, 0);

    v2 = v2 - touchStartPos_;
    if (endTime - startTime_ < kMaxTapTime  &&
        v2.Length() < kMaxTapDistance) {
      LOGI("Detected a tap event, (%f, %f)", touchStartPos_.x, touchStartPos_.y);
      ProcessTapEvent(static_cast<int>(touchStartPos_.x),
                      static_cast<int>(touchStartPos_.y));
      return true;
    }

    if (v2.Length() < kMinDistance) {
      LOGI("---- too short to be considered a swipe");
      return true;
    }

    if (std::abs(v2.x) > std::abs(v2.y)) {
      ResetUserEventCache();
      return true;
    }

    int offset = 1;
    if (v2.y > 0) {
      // swiping down, decrease the index...
      offset = -1;
    }

    uint32_t idx = textureIdx_;
    idx += offset + textures_.size();
    textureIdx_ = idx % textures_.size();

    UpdateUI();
  }

  return true;
}

void ImageViewEngine::EnableWelcomeUI(void) {
  JNIEnv* jni;
  app_->activity->vm->AttachCurrentThread(&jni, NULL);

  // Default class retrieval
  jclass clazz = jni->GetObjectClass(app_->activity->clazz);
  jmethodID methodID = jni->GetMethodID(clazz, "EnableUI", "(I)V");
  jni->CallVoidMethod(app_->activity->clazz, methodID,
                     renderModeBits_);

  app_->activity->vm->DetachCurrentThread();
}

void ImageViewEngine::EnableRenderUI(void) {
  JNIEnv* jni;
  app_->activity->vm->AttachCurrentThread(&jni, NULL);

  // Default class retrieval
  jclass clazz = jni->GetObjectClass(app_->activity->clazz);
  jmethodID methodID = jni->GetMethodID(clazz, "EnableRenderUI",
                                        "(Ljava/lang/String;)V");

  jstring file = jni->NewStringUTF(textures_[textureIdx_]->Name().c_str());
  jni->CallVoidMethod(app_->activity->clazz, methodID, file);
  jni->DeleteLocalRef(file);

  app_->activity->vm->DetachCurrentThread();
}
void ImageViewEngine::UpdateUI(void) {
  JNIEnv* jni;
  app_->activity->vm->AttachCurrentThread(&jni, NULL);

  // Default class retrieval
  jclass clazz = jni->GetObjectClass(app_->activity->clazz);
  jmethodID methodID = jni->GetMethodID(clazz, "UpdateUI",
                                        "(ILjava/lang/String;)V");
  jstring file = jni->NewStringUTF(textures_[textureIdx_]->Name().c_str());
  jni->CallVoidMethod(app_->activity->clazz, methodID,
                     renderModeBits_, file);
  jni->DeleteLocalRef(file);

  app_->activity->vm->DetachCurrentThread();
}
