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

#ifndef __APP_ENGINE_H__
#define __APP_ENGINE_H__

#include <atomic>
#include <mutex>
#include <condition_variable>
#include <string>

#include <initializer_list>
#include <memory>
#include <vector>
#include <cstdlib>
#include <cassert>
#include <android/sensor.h>
#include <android_native_app_glue.h>

#include "common.h"
#include "android_debug.h"
#include "gldebug.h"
#include "ShaderProgram.h"
#include "AssetTexture.h"

class ImageViewEngine {
public:
  ImageViewEngine(struct android_app* app);

  bool InitializeDisplay(void);
  void TerminateDisplay(void);

  // Touch/swipe event handler
  bool ProcessInputEvent(const AInputEvent* event);

  void DrawFrame(void);
  bool GetAnimationStatus(void);
  void EnableAnimation(bool enable);

  ASensorManager* sensorManager;
  const ASensor* accelerometerSensor;
  ASensorEventQueue* sensorEventQueue;

private:
  struct android_app* app_;

  int animating_;
  EGLDisplay display_;
  EGLSurface surface_;
  DISPLAY_COLORSPACE dispColorSpace_;
  DISPLAY_FORMAT dispFormat_;

  EGLContext eglContext_;
  int32_t renderTargetWidth_;
  int32_t renderTargetHeight_;

  ShaderProgram program_;

  // Image file texture store
  std::vector<AssetTexture*> textures_;
  std::atomic<uint32_t>  textureIdx_;

  enum WIDECOLOR_MODE {
    P3_PASSTHROUGH_R8G8B8A8_REV,
    P3_PASSTHROUGH_R10G10B10A2_REV,
    P3_PASSTHROUGH_FP16,
    P3_R8G8B8A8_REV,
    P3_R10G10B10A2_REV,
    P3_FP16,
    SRGBA_R8G8B8A8_REV,
    MODE_COUNT
  };
  bool CreateWideColorCtx(void);
  bool CreateWideColorCtx(WIDECOLOR_MODE mode);
  void DestroyWideColorCtx(void);

  bool CreateTextures(void);
  void DeleteTextures(void);

  uint32_t renderModeBits_;

  // Input event data
  mathfu::vec2 touchStartPos_;
  uint64_t startTime_;
  void ResetUserEventCache(void);
  void ProcessTapEvent(int x, int y);

  // 2 TextView UI management
  void EnableWelcomeUI(void);
  void EnableRenderUI(void);
  void UpdateUI(void);

};


#endif // __APP_ENGINE_H__
