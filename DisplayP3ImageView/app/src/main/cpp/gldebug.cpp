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

#include <vector>
#include <cassert>
#include <memory>
#include <map>
#include <android/log.h>
#include <GLES3/gl32.h>
#include <GLES3/gl3ext.h>

#include "android_debug.h"
#include "gldebug.h"

/*
 * eglConfig ( swapchain ) capability
 */
void PrintEglConfig(void) {
  EGLint numConfigs;
  EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

  eglInitialize(display, 0, 0);
  eglGetConfigs(display, nullptr, 0, &numConfigs);
  std::vector<EGLConfig> supportedConfigs(numConfigs);
  eglGetConfigs(display, supportedConfigs.data(), numConfigs, &numConfigs);
  assert(numConfigs);
  for (auto cfg : supportedConfigs) {
    PrintEglConfig(display, cfg);
  }
}

#define UKNOWN_TAG  "UNKNOW_TAG"
#define MAKE_PAIR(val) std::make_pair(val, #val)
template <typename T>
const char* GetPairStr(T key, std::vector<std::pair<T, const char*>>& store) {
  typedef typename std::vector<std::pair<T, const char*> >::iterator iterator;
  for (iterator it = store.begin(); it != store.end(); ++it) {
    if (it->first == key) {
      return it->second;
    }
  }
  LOGW("(%#08x) : UNKNOWN_TAG for %s", key, typeid(store[0].first).name());
  return UKNOWN_TAG;
}

using UINT32_PAIR = std::pair<uint32_t, const char*>;
std::vector<UINT32_PAIR> eglSurfaceTypeInfo{
    MAKE_PAIR(EGL_PBUFFER_BIT),
    MAKE_PAIR(EGL_PIXMAP_BIT),
    MAKE_PAIR(EGL_WINDOW_BIT),
    MAKE_PAIR(EGL_VG_COLORSPACE_LINEAR_BIT),
    MAKE_PAIR(EGL_VG_ALPHA_FORMAT_PRE_BIT),
    MAKE_PAIR(EGL_MULTISAMPLE_RESOLVE_BOX_BIT),
    MAKE_PAIR(EGL_SWAP_BEHAVIOR_PRESERVED_BIT)
};
const char* GetEGLSurfaceTypeStr(uint32_t mask) {
  return GetPairStr<uint32_t>(mask, eglSurfaceTypeInfo);
}

void PrintEglConfig(EGLDisplay display, EGLConfig cfg) {
  EGLint r, g, b, d, a, s;
  eglGetConfigAttrib(display, cfg, EGL_RED_SIZE, &r);
  eglGetConfigAttrib(display, cfg, EGL_GREEN_SIZE, &g);
  eglGetConfigAttrib(display, cfg, EGL_BLUE_SIZE, &b);
  eglGetConfigAttrib(display, cfg, EGL_ALPHA_SIZE, &a);
  eglGetConfigAttrib(display, cfg, EGL_DEPTH_SIZE, &d);
  eglGetConfigAttrib(display, cfg, EGL_SURFACE_TYPE, &s);
  LOGI("(%d, %d, %d, %d) and (%d) (%08x)", r, g, b, a, d, s);
  for (auto& pair : eglSurfaceTypeInfo) {
     if (pair.first & s ) {
       LOGI("SURFACE_TYPE: %s", GetEGLSurfaceTypeStr(pair.first));
     }
  }
}

/*
 * OpenGL Implementation Info
 */
std::vector<UINT32_PAIR> OpenGLInfo {
    MAKE_PAIR(GL_VENDOR),
    MAKE_PAIR(GL_RENDERER),
    MAKE_PAIR(GL_VERSION),
    MAKE_PAIR(GL_SHADING_LANGUAGE_VERSION),
    MAKE_PAIR(GL_EXTENSIONS),
};
void PrintGLInfo(void) {
  for (auto name : OpenGLInfo) {
    LOGI("OpenGL %s: %s", name.second, glGetString(name.first));
  }
}

static std::vector<UINT32_PAIR> EGLInfo {
    MAKE_PAIR(EGL_CLIENT_APIS),
    MAKE_PAIR(EGL_VENDOR),
    MAKE_PAIR(EGL_VERSION),
    MAKE_PAIR(EGL_EXTENSIONS),
};
void PrintEGLInfo(EGLDisplay disp) {
  for (auto name : EGLInfo) {
    LOGI("OpenGL %s: %s", name.second, eglQueryString(disp, name.first));
  }
}

std::vector<UINT32_PAIR> glErrorInfo {
    MAKE_PAIR(GL_NO_ERROR),
    MAKE_PAIR(GL_INVALID_ENUM),
    MAKE_PAIR(GL_INVALID_VALUE),
    MAKE_PAIR(GL_INVALID_OPERATION),
    MAKE_PAIR(GL_OUT_OF_MEMORY),
};
const char* GetGLErrorStr(uint32_t errorCode) {
  return GetPairStr(errorCode, glErrorInfo);
}


/*
 * Matrix helping functions
 */
void PrintMatrix(mathfu::mat3& matrix) {
  char buf[64] = {0,};
  int index;
  for (int r = 0; r < matrix.kRows; r++) {
    index = 0;
    index += snprintf(&buf[index], 64 - index, "Matrix: ");
    for (int c= 0; c < matrix.kColumns; c++)
      index += snprintf(&buf[index], 64 - index, "%f  ", matrix(r, c));

    LOGI("%s", buf);
  }
}

