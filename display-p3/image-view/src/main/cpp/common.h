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

#ifndef __COMMON_H__
#define __COMMON_H__

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl32.h>
#include <GLES3/gl3ext.h>

/*
 * App Context Format
 */
enum DISPLAY_COLORSPACE {
  P3_PASSTHROUGH,
  P3,
  SRGB,
  COUNT,
  INVALID = COUNT,
};

enum DISPLAY_FORMAT {
  R10G10B10_A2_REV,
  R8G8B8A8_REV,
  RGBA_FP16,
  FORMAT_COUNT,
  INVALID_FORMAT = FORMAT_COUNT
};

// Rendering Mode BitMask
#define RENDERING_P3   0x01
#define RENDERING_SRGB 0x02

#define DEFAULT_DISPLAY_GAMMA  (1.0f/2.2f)
#define DEFAULT_P3_IMAGE_GAMMA (1.0f/2.2f)

// EGL Bits
#ifndef EGL_OPENGL_ES3_BITS
#define EGL_OPENGL_ES3_BIT 0x00000040
#endif
#ifndef EGL_GL_COLORSPACE_DISPLAY_P3_EXT
#define EGL_GL_COLORSPACE_DISPLAY_P3_EXT 0x3363
#endif

#endif // __COMMON_H__
