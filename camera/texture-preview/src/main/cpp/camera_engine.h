
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
 */

#ifndef __CAMERA_ENGINE_H__
#define __CAMERA_ENGINE_H__
#include <jni.h>
#include <android/native_activity.h>
#include <android/native_window_jni.h>
#include <android/native_window.h>
#include <functional>
#include <thread>

#include "camera_manager.h"

/**
 * basic CameraAppEngine
 */
class CameraEngine {
 public:
  //
  explicit CameraEngine(JNIEnv* env, jobject surface) :
  surface_(surface),
      camera_(nullptr),
      env_(env),
      requestHeight_(0),
      requestWidth_(0),
      rotation_ (0) { env->GetJavaVM(&vm_); }
  explicit CameraEngine(JNIEnv* env, jint w, jint h, jint rotation):
    surface_(nullptr),
    camera_(nullptr),
    env_ (env),
    requestHeight_(h),
    requestWidth_(w),
    rotation_ (rotation) { env->GetJavaVM(&vm_);}

    ~CameraEngine() { DeleteCamera(); }

    ANativeWindow* getNativeWin(void) {
      return ANativeWindow_fromSurface(env_, surface_);
  }
  // end newly added things...

  // Manage NDKCamera Object
  void CreateCamera(void);
  void CreateCameraSession(jobject surface) {
    surface_ = surface;
    camera_->CreateSession(getNativeWin());
  }
  void DeleteCamera(void);

  void StartPreview(bool start);
  const ImageFormat& GetCompatibleCameraRes() const {
    return compatibleCameraRes_;
  }
  int32_t GetCameraSensorOrientation(int32_t facing);

 private:
  int GetDisplayRotation(void);
  NDKCamera* camera_;

  JNIEnv* env_;
  JavaVM* vm_;
  int     rotation_;
  jobject surface_;
  int32_t requestWidth_;
  int32_t requestHeight_;
  ImageFormat  compatibleCameraRes_;
};

/**
 * retrieve global singleton CameraEngine instance
 * @return the only instance of CameraEngine in the app
 */
CameraEngine* GetAppEngine(void);

#endif  // __CAMERA_ENGINE_H__
