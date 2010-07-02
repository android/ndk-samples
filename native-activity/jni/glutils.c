/*
 * Copyright (C) 2010 The Android Open Source Project
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

#include "glutils.h"

const char *EGLstrerror(EGLint err) {
    switch (err) {
        case EGL_SUCCESS:           return "EGL_SUCCESS";
        case EGL_NOT_INITIALIZED:   return "EGL_NOT_INITIALIZED";
        case EGL_BAD_ACCESS:        return "EGL_BAD_ACCESS";
        case EGL_BAD_ALLOC:         return "EGL_BAD_ALLOC";
        case EGL_BAD_ATTRIBUTE:     return "EGL_BAD_ATTRIBUTE";
        case EGL_BAD_CONFIG:        return "EGL_BAD_CONFIG";
        case EGL_BAD_CONTEXT:       return "EGL_BAD_CONTEXT";
        case EGL_BAD_CURRENT_SURFACE: return "EGL_BAD_CURRENT_SURFACE";
        case EGL_BAD_DISPLAY:       return "EGL_BAD_DISPLAY";
        case EGL_BAD_MATCH:         return "EGL_BAD_MATCH";
        case EGL_BAD_NATIVE_PIXMAP: return "EGL_BAD_NATIVE_PIXMAP";
        case EGL_BAD_NATIVE_WINDOW: return "EGL_BAD_NATIVE_WINDOW";
        case EGL_BAD_PARAMETER:     return "EGL_BAD_PARAMETER";
        case EGL_BAD_SURFACE:       return "EGL_BAD_SURFACE";
        case EGL_CONTEXT_LOST:      return "EGL_CONTEXT_LOST";
        default: return "UNKNOWN";
    }
}

int32_t selectConfigForPixelFormat(EGLDisplay dpy, EGLint const* attrs,
        int32_t format, EGLConfig* outConfig) {
    EGLint numConfigs = -1, n=0;

    if (!attrs)
        return -1;

    if (outConfig == NULL)
        return -1;
    
    int fbSzA, fbSzR, fbSzG, fbSzB;
    switch (format) {
        case WINDOW_FORMAT_RGBA_8888:
            fbSzA = fbSzR = fbSzG = fbSzB = 8;
            break;
        case WINDOW_FORMAT_RGBX_8888:
            fbSzA = 0; fbSzR = fbSzG = fbSzB = 8;
            break;
        case WINDOW_FORMAT_RGB_565:
            fbSzA = 0; fbSzR = 5; fbSzG = 6; fbSzB = 5;
            break;
        default:
            LOGW("Unknown format");
            return -1;
    }

    // Get all the "potential match" configs...
    if (eglGetConfigs(dpy, NULL, 0, &numConfigs) == EGL_FALSE) {
        LOGW("eglGetConfigs failed");
        return -1;
    }

    EGLConfig* const configs = (EGLConfig*)malloc(sizeof(EGLConfig)*numConfigs);
    if (eglChooseConfig(dpy, attrs, configs, numConfigs, &n) == EGL_FALSE) {
        free(configs);
        LOGW("eglChooseConfig failed");
        return -1;
    }

    int i;
    EGLConfig config = NULL;
    for (i=0 ; i<n ; i++) {
        EGLint r,g,b,a;
        EGLConfig curr = configs[i];
        eglGetConfigAttrib(dpy, curr, EGL_RED_SIZE,   &r);
        eglGetConfigAttrib(dpy, curr, EGL_GREEN_SIZE, &g);
        eglGetConfigAttrib(dpy, curr, EGL_BLUE_SIZE,  &b);
        eglGetConfigAttrib(dpy, curr, EGL_ALPHA_SIZE, &a);
        if (fbSzA == a && fbSzR == r && fbSzG == g && fbSzB  == b) {
            config = curr;
            break;
        }
    }

    free(configs);
    
    if (i<n) {
        *outConfig = config;
        return 0;
    }

    LOGW("No config with desired pixel format");
    return -1;
}

int32_t selectConfigForNativeWindow(EGLDisplay dpy, EGLint const* attrs,
        EGLNativeWindowType window, EGLConfig* outConfig) {
    int err;
    int format;
    
    if (!window)
        return -1;
    
    return selectConfigForPixelFormat(dpy, attrs,
            ANativeWindow_getFormat(window), outConfig);
}

