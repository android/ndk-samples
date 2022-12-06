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

#ifndef _NDKSUPPORT_H
#define _NDKSUPPORT_H

#define NDK_HELPER_VERSION "0.90"

/******************************************************************
 * NDK support helpers
 * Utility module to provide misc functionalities that is used widely in native
 * applications,
 * such as gesture detection, jni bridge, openGL context etc.
 *
 * The purpose of this module is,
 * - Provide best practices using NDK
 * - Provide handy utility functions for NDK development
 * - Make NDK samples more simpler and readable
 */
#include "GLContext.h"        // EGL & OpenGL manager
#include "JNIHelper.h"        // JNI support
#include "gestureDetector.h"  // Tap/Doubletap/Pinch detector
#include "gl3stub.h"          // GLES3 stubs
#include "interpolator.h"     // Interpolator
#include "perfMonitor.h"      // FPS counter
#include "sensorManager.h"    // SensorManager
#include "shader.h"           // Shader compiler support
#include "tapCamera.h"        // Tap/Pinch camera control
#include "vecmath.h"  // Vector math support, C++ implementation n current version
#endif
