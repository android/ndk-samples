
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

#include <android/native_window.h>
#include <android_native_app_glue.h>
#include <functional>
#include <thread>

#include "camera_manager.h"

/**
 * basic CameraAppEngine
 */
class CameraEngine {
 public:
  explicit CameraEngine(android_app* app);
  ~CameraEngine();

  // Interfaces to android application framework
  struct android_app* AndroidApp(void) const;
  void OnAppInitWindow(void);
  void DrawFrame(void);
  void OnAppConfigChange(void);
  void OnAppTermWindow(void);

  // Native Window handlers
  int32_t GetSavedNativeWinWidth(void);
  int32_t GetSavedNativeWinHeight(void);
  int32_t GetSavedNativeWinFormat(void);
  void SaveNativeWinRes(int32_t w, int32_t h, int32_t format);

  // UI handlers
  void RequestCameraPermission();
  void OnCameraPermission(jboolean granted);
  void EnableUI(void);
  void OnTakePhoto(void);
  void OnCameraParameterChanged(int32_t code, int64_t val);

  // Manage NDKCamera Object
  void CreateCamera(void);
  void DeleteCamera(void);

 private:
  void OnPhotoTaken(const char* fileName);
  int  GetDisplayRotation(void);

  struct android_app* app_;
  ImageFormat savedNativeWinRes_;
  bool cameraGranted_;
  int rotation_;
  volatile bool cameraReady_;
  NDKCamera* camera_;
  ImageReader* yuvReader_;
  ImageReader* jpgReader_;
};

/**
 * retrieve global singleton CameraEngine instance
 * @return the only instance of CameraEngine in the app
 */
CameraEngine* GetAppEngine(void);

#endif  // __CAMERA_ENGINE_H__
