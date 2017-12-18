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
// GLContext.h
//--------------------------------------------------------------------------------
#ifndef GLCONTEXT_H_
#define GLCONTEXT_H_

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <android/log.h>

#include "JNIHelper.h"

namespace ndk_helper {

//--------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------

//--------------------------------------------------------------------------------
// Class
//--------------------------------------------------------------------------------

/******************************************************************
 * OpenGL context handler
 * The class handles OpenGL and EGL context based on Android activity life cycle
 * The caller needs to call corresponding methods for each activity life cycle
 *events as it's done in sample codes.
 *
 * Also the class initializes OpenGL ES3 when the compatible driver is installed
 *in the device.
 * getGLVersion() returns 3.0~ when the device supports OpenGLES3.0
 *
 * Thread safety: OpenGL context is expecting used within dedicated single
 *thread,
 * thus GLContext class is not designed as a thread-safe
 */
class GLContext {
 private:
  // EGL configurations
  ANativeWindow* window_;
  EGLDisplay display_;
  EGLSurface surface_;
  EGLContext context_;
  EGLConfig config_;

  // Screen parameters
  int32_t screen_width_;
  int32_t screen_height_;
  int32_t color_size_;
  int32_t depth_size_;

  // Flags
  bool gles_initialized_;
  bool egl_context_initialized_;
  bool es3_supported_;
  float gl_version_;
  bool context_valid_;

  void InitGLES();
  void Terminate();
  bool InitEGLSurface();
  bool InitEGLContext();

  GLContext(GLContext const&);
  void operator=(GLContext const&);
  GLContext();
  virtual ~GLContext();

 public:
  static GLContext* GetInstance() {
    // Singleton
    static GLContext instance;

    return &instance;
  }

  bool Init(ANativeWindow* window);
  EGLint Swap();
  bool Invalidate();

  void Suspend();
  EGLint Resume(ANativeWindow* window);

  ANativeWindow* GetANativeWindow(void) const { return window_; };
  int32_t GetScreenWidth() const { return screen_width_; }
  int32_t GetScreenHeight() const { return screen_height_; }

  int32_t GetBufferColorSize() const { return color_size_; }
  int32_t GetBufferDepthSize() const { return depth_size_; }
  float GetGLVersion() const { return gl_version_; }
  bool CheckExtension(const char* extension);

  EGLDisplay GetDisplay() const { return display_; }
  EGLSurface GetSurface() const { return surface_; }
};

}  // namespace ndkHelper

#endif /* GLCONTEXT_H_ */
