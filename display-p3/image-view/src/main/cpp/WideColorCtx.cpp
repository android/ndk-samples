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
#include "ImageViewEngine.h"
#include "android_debug.h"

struct APP_WIDECOLOR_MODE_CFG {
  DISPLAY_COLORSPACE space_;
  DISPLAY_FORMAT fmt_;
};
struct GL_WIDECOLOR_MODE_CFG {
  EGLint  space_;
  EGLint  r_, g_, b_, a_;
};

static const APP_WIDECOLOR_MODE_CFG appWideColorCfg[] = {
    { DISPLAY_COLORSPACE::P3, DISPLAY_FORMAT::R8G8B8A8_REV },
    { DISPLAY_COLORSPACE::P3, DISPLAY_FORMAT::R10G10B10_A2_REV,},
    { DISPLAY_COLORSPACE::P3, DISPLAY_FORMAT::RGBA_FP16},
    { DISPLAY_COLORSPACE::SRGB,DISPLAY_FORMAT::R8G8B8A8_REV},
};
static GL_WIDECOLOR_MODE_CFG glWideColorCfg[] = {
    {
        EGL_GL_COLORSPACE_DISPLAY_P3_EXT,
        8, 8, 8, 8
    },
    {
        EGL_GL_COLORSPACE_DISPLAY_P3_EXT,
        10, 10, 10, 2
    },
    {
        EGL_GL_COLORSPACE_DISPLAY_P3_EXT,
        16, 16, 16, 16
    },
    {
        EGL_GL_COLORSPACE_SRGB_KHR,
        8, 8, 8, 8
    },
};

#define EGL_P3_EXT     "EGL_EXT_gl_colorspace_display_p3"
#define EGL_COLORSPACE_EXT "EGL_KHR_gl_colorspace"
static bool CheckRequiredEGLExt(EGLDisplay disp, std::vector<std::string>& exts) {
  std::string eglExt(eglQueryString(disp, EGL_EXTENSIONS)) ;
  for(auto& ext : exts) {
    std::string::size_type extPos = eglExt.find(ext);
    if (extPos == std::string::npos) {
      return false;
    }
  }
  return true;
}


/*
 * Initialize an EGL eglContext_ for the current display_.
 *
 * Supported Format:
 *     8888:     EGL_COLOR_COMPONENT_TYPE_EXT  EGL_COLOR_COMPONENT_TYPE_FIXED_EXT
 *     101010102:EGL_COLOR_COMPONENT_TYPE_EXT  EGL_COLOR_COMPONENT_TYPE_FIXED_EXT
 *     16161616: EGL_COLOR_COMPONENT_TYPE_EXT  EGL_COLOR_COMPONENT_TYPE_FLOAT_EXT
 *
 */
bool ImageViewEngine::CreateWideColorCtx(WIDECOLOR_MODE mode) {
  EGLBoolean status;

  std::vector<EGLint> attributes {
      EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
      EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
      EGL_BLUE_SIZE,  glWideColorCfg[mode].b_,
      EGL_GREEN_SIZE, glWideColorCfg[mode].g_,
      EGL_RED_SIZE,   glWideColorCfg[mode].r_,
      EGL_ALPHA_SIZE, glWideColorCfg[mode].a_,
  };

  // for RGBA888, still set to EGL_COLOR_COMPONENT_TYPE_FIXED_EXT?
  if (mode == WIDECOLOR_MODE::P3_R10G10B10A2_REV) {
    attributes.push_back(EGL_COLOR_COMPONENT_TYPE_EXT);
    attributes.push_back(EGL_COLOR_COMPONENT_TYPE_FLOAT_EXT);
  }
  attributes.push_back(EGL_NONE);

  // request just one matching config and use it
  EGLint    cfgCount = 1;
  EGLConfig config;
  status = eglChooseConfig(display_, attributes.data(), &config,
                           cfgCount, &cfgCount);
  if (!status || cfgCount != 1) {
    // if not support, report to caller so caller could choose another one
    LOGI("==== Chosen Config type(%d) is not supported", mode);
    return false;
  }

  // Create GL3 Context
  attributes.resize(0);
  attributes.push_back(EGL_CONTEXT_CLIENT_VERSION);
  attributes.push_back(3);
  attributes.push_back(EGL_NONE);
  eglContext_ = eglCreateContext(display_, config,
                                 EGL_NO_CONTEXT, attributes.data());
  if(eglContext_ == EGL_NO_CONTEXT) {
    return false;
  }

  EGLint format;
  eglGetConfigAttrib(display_, config, EGL_NATIVE_VISUAL_ID, &format);
  int32_t res = ANativeWindow_setBuffersGeometry(app_->window, 0, 0, format);
  if(res < 0) {
    eglDestroyContext(display_, eglContext_);
    eglContext_ = EGL_NO_CONTEXT;
    return false;
  }

  // Create Surface, which will turn on Display P3 wide gamut mode
  attributes.resize(0);
  attributes.push_back(EGL_GL_COLORSPACE_KHR);
  attributes.push_back(glWideColorCfg[mode].space_);
  attributes.push_back(EGL_NONE);
  surface_ = eglCreateWindowSurface(
      display_, config, app_->window, attributes.data());
  if(surface_ == EGL_NO_SURFACE) {
    LOGI("====Surface for mode (%d) is not supported", mode);
    eglDestroyContext(display_, eglContext_);
    eglContext_ = EGL_NO_CONTEXT;
    return false;
  }
  status = eglMakeCurrent(display_, surface_,
                          surface_, eglContext_);
  ASSERT(status, "eglMakeCurrent() Failed");

  dispColorSpace_ = appWideColorCfg[mode].space_;
  dispFormat_ = appWideColorCfg[mode].fmt_;

  eglQuerySurface(display_, surface_, EGL_WIDTH, &renderTargetWidth_);
  eglQuerySurface(display_, surface_, EGL_HEIGHT, &renderTargetHeight_);

  return true;
}

bool ImageViewEngine::CreateWideColorCtx(void) {
  EGLint major, minor;
  display_ = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  eglInitialize(display_, &major, &minor);

  /*
   * Display-P3 needs EGL_EXT_gl_colorspace_display_p3 extension
   * which needs EGL 1.4. If not available, Display P3 is not supported
   * in that case, create legacy RGBA8888 eglContext_.
   */
  std::vector<std::string> exts {
      EGL_COLORSPACE_EXT,
      EGL_P3_EXT
  };

  if ((major <= 1 && minor < 4) ||
      !CheckRequiredEGLExt(display_, exts)) {
    LOGW("====Warning: Display P3 is not supported,"
             "creating legacy mode GL Context");
    return CreateWideColorCtx(SRGBA_R8G8B8A8_REV);
  }

  // Proceed to the wide color gamut modes
  WIDECOLOR_MODE modes[] = {
      P3_R8G8B8A8_REV,
      P3_R10G10B10A2_REV,
      P3_FP16,
      SRGBA_R8G8B8A8_REV,
  };
  for (auto mode : modes) {
    if (CreateWideColorCtx(mode))
      return true;
  }
  return false;
}

void ImageViewEngine::DestroyWideColorCtx() {
  if (display_ == EGL_NO_DISPLAY) {
    return;
  }

  eglMakeCurrent(display_, EGL_NO_SURFACE,
                   EGL_NO_SURFACE, EGL_NO_CONTEXT);

  if (eglContext_ != EGL_NO_CONTEXT) {
    eglDestroyContext(display_, eglContext_);
  }
  if (surface_ != EGL_NO_SURFACE) {
    eglDestroySurface(display_, surface_);
  }
  eglTerminate(display_);

  display_ = EGL_NO_DISPLAY;
  eglContext_ = EGL_NO_CONTEXT;
  surface_ = EGL_NO_SURFACE;
  dispColorSpace_ = DISPLAY_COLORSPACE::INVALID;
  dispFormat_ = DISPLAY_FORMAT::INVALID_FORMAT;
}
