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

#ifndef __ANDROID_GL_DEBUG_H__
#define __ANDROID_GL_DEBUG_H__

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl32.h>
#include "android_debug.h"
#include "mathfu/glsl_mappings.h"
void PrintEGLInfo(EGLDisplay disp);
void PrintEglConfig(void);
void PrintEglConfig(EGLDisplay display, EGLConfig cfg);
const char* GetGLErrorStr(uint32_t errorCode);
void PrintGLInfo(void);

inline void CheckGlError(const char* op) {
  for (GLint error = glGetError(); error; error
                                              = glGetError()) {
    LOGI("===== after %s() glError (%#x): %s\n", op, error, GetGLErrorStr(error));
  }
}

void PrintMatrix(mathfu::mat3& matrix);

#endif //ANDROID_GL_DEBUG
