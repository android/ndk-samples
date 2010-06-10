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

#include <string.h>
#include <jni.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __VA_ARGS__))

static void onDestroy(android_activity_t* activity)
{
    LOGI("Destroy: %p\n", activity);
}

static void onStart(android_activity_t* activity)
{
    LOGI("Start: %p\n", activity);
}

static void onResume(android_activity_t* activity)
{
    LOGI("Resume: %p\n", activity);
}

static void* onSaveInstanceState(android_activity_t* activity, size_t* outLen)
{
    LOGI("SaveInstanceState: %p\n", activity);
    return NULL;
}

static void onPause(android_activity_t* activity)
{
    LOGI("Pause: %p\n", activity);
}

static void onStop(android_activity_t* activity)
{
    LOGI("Stop: %p\n", activity);
}

static void onLowMemory(android_activity_t* activity)
{
    LOGI("LowMemory: %p\n", activity);
}

static void onWindowFocusChanged(android_activity_t* activity, int focused)
{
    LOGI("WindowFocusChanged: %p -- %d\n", activity, focused);
}

static void onSurfaceCreated(android_activity_t* activity, android_surface_t* surface)
{
    LOGI("SurfaceCreated: %p -- %p\n", activity, surface);
}

static void onSurfaceChanged(android_activity_t* activity, android_surface_t* surface,
        int format, int width, int height)
{
    LOGI("SurfaceChanged: %p -- %p fmt=%d w=%d h=%d\n", activity, surface,
            format, width, height);
}

static void onSurfaceDestroyed(android_activity_t* activity, android_surface_t* surface)
{
    LOGI("SurfaceDestroyed: %p -- %p\n", activity, surface);
}

void android_onCreateActivity(android_activity_t* activity,
        void* savedState, size_t savedStateSize)
{
    LOGI("Creating: %p\n", activity);
    activity->callbacks->onDestroy = onDestroy;
    activity->callbacks->onStart = onStart;
    activity->callbacks->onResume = onResume;
    activity->callbacks->onSaveInstanceState = onSaveInstanceState;
    activity->callbacks->onPause = onPause;
    activity->callbacks->onStop = onStop;
    activity->callbacks->onLowMemory = onLowMemory;
    activity->callbacks->onWindowFocusChanged = onWindowFocusChanged;
    activity->callbacks->onSurfaceCreated = onSurfaceCreated;
    activity->callbacks->onSurfaceChanged = onSurfaceChanged;
    activity->callbacks->onSurfaceDestroyed = onSurfaceDestroyed;
}
