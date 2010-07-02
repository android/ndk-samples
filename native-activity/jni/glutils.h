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

#include <android/native_activity.h>
#include <android/log.h>

#include <EGL/egl.h>
#include <GLES/gl.h>
#include <GLES/glext.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "native-activity", __VA_ARGS__))

extern const char *EGLstrerror(EGLint err);
extern int32_t selectConfigForPixelFormat(EGLDisplay dpy, EGLint const* attrs,
        int32_t format, EGLConfig* outConfig);
extern int32_t selectConfigForNativeWindow(EGLDisplay dpy, EGLint const* attrs,
        EGLNativeWindowType window, EGLConfig* outConfig);
