/*
 * Copyright (C) Google Inc.
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
#ifndef endlesstunnel_common_hpp
#define endlesstunnel_common_hpp

extern "C" {
    #include <EGL/egl.h>
    #include <GLES2/gl2.h>
    #include <jni.h>
    #include <errno.h>
    #include <android/sensor.h>
    #include <android/log.h>
    #include <android_native_app_glue.h>
    #include <unistd.h>
    #include <stdlib.h>
}
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"

#define DEBUG_TAG "EndlessTunnel:Native"
#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, __VA_ARGS__))
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, DEBUG_TAG, __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, DEBUG_TAG, __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, DEBUG_TAG, __VA_ARGS__))
#define ABORT_GAME { LOGE("*** GAME ABORTING."); *((volatile char*)0) = 'a'; }
#define DEBUG_BLIP LOGD("[ BLIP ]: %s:%d", __FILE__, __LINE__)

#define MY_ASSERT(cond) { if (!(cond)) { LOGE("ASSERTION FAILED: %s", #cond); \
   ABORT_GAME; } }

#define BUFFER_OFFSET(i) ((char*)NULL + (i))

#include "our_key_codes.hpp"

#endif

