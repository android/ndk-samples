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

#include <math.h>
#include "sensorManager.h"

//--------------------------------------------------------------------------------
// sensorManager.cpp
//--------------------------------------------------------------------------------
namespace ndk_helper {

//--------------------------------------------------------------------------------
// includes
//--------------------------------------------------------------------------------

//-------------------------------------------------------------------------
// Sensor handlers
//-------------------------------------------------------------------------
SensorManager::SensorManager()
    : sensorManager_(nullptr),
      accelerometerSensor_(nullptr),
      sensorEventQueue_(nullptr) {}

SensorManager::~SensorManager() {}

void SensorManager::Init(android_app *app) {
  sensorManager_ = AcquireASensorManagerInstance(app);
  accelerometerSensor_ = ASensorManager_getDefaultSensor(
      sensorManager_, ASENSOR_TYPE_ACCELEROMETER);
  sensorEventQueue_ = ASensorManager_createEventQueue(
      sensorManager_, app->looper, LOOPER_ID_USER, NULL, NULL);
}

void SensorManager::Process(const int32_t id) {
  // If a sensor has data, process it now.
  if (id == LOOPER_ID_USER) {
    if (accelerometerSensor_ != NULL) {
      ASensorEvent event;
      while (ASensorEventQueue_getEvents(sensorEventQueue_, &event, 1) > 0) {
        float maginitude = event.acceleration.x * event.acceleration.x +
                           event.acceleration.y * event.acceleration.y;
        if (maginitude * 4 >= event.acceleration.z * event.acceleration.z) {
          int32_t orientation = ORIENTATION_REVERSE_LANDSCAPE;
          float angle = atan2f(-event.acceleration.y, event.acceleration.x);
          if (angle <= -M_PI_2 - M_PI_4) {
            orientation = ORIENTATION_REVERSE_LANDSCAPE;
          } else if (angle <= -M_PI_4) {
            orientation = ORIENTATION_PORTRAIT;
          } else if (angle <= M_PI_4) {
            orientation = ORIENTATION_LANDSCAPE;
          } else if (angle <= M_PI_2 + M_PI_4) {
            orientation = ORIENTATION_REVERSE_PORTRAIT;
          }

          //					LOGI( "orientation %f %d", angle,
          //orientation);
        }
      }
    }
  }
}

void SensorManager::Resume() {
  // When the app gains focus, start monitoring the accelerometer.
  if (accelerometerSensor_ != NULL) {
    ASensorEventQueue_enableSensor(sensorEventQueue_, accelerometerSensor_);
    // We'd like to get 60 events per second (in us).
    ASensorEventQueue_setEventRate(sensorEventQueue_, accelerometerSensor_,
                                   (1000L / 60) * 1000);
  }
}

void SensorManager::Suspend() {
  // When the app loses focus, stop monitoring the accelerometer.
  // This is to avoid consuming battery while not being used.
  if (accelerometerSensor_ != NULL) {
    ASensorEventQueue_disableSensor(sensorEventQueue_, accelerometerSensor_);
  }
}

#include <dlfcn.h>
ASensorManager* AcquireASensorManagerInstance(android_app* app) {

  if(!app)
    return nullptr;

  typedef ASensorManager *(*PF_GETINSTANCEFORPACKAGE)(const char *name);
  void* androidHandle = dlopen("libandroid.so", RTLD_NOW);
  PF_GETINSTANCEFORPACKAGE getInstanceForPackageFunc = (PF_GETINSTANCEFORPACKAGE)
      dlsym(androidHandle, "ASensorManager_getInstanceForPackage");
  if (getInstanceForPackageFunc) {
    JNIEnv* env = nullptr;
    app->activity->vm->AttachCurrentThread(&env, NULL);

    jclass android_content_Context = env->GetObjectClass(app->activity->clazz);
    jmethodID midGetPackageName = env->GetMethodID(android_content_Context,
                                                   "getPackageName",
                                                   "()Ljava/lang/String;");
    jstring packageName= (jstring)env->CallObjectMethod(app->activity->clazz,
                                                        midGetPackageName);

    const char *nativePackageName = env->GetStringUTFChars(packageName, 0);
    ASensorManager* mgr = getInstanceForPackageFunc(nativePackageName);
    env->ReleaseStringUTFChars(packageName, nativePackageName);
    app->activity->vm->DetachCurrentThread();
    if (mgr) {
      dlclose(androidHandle);
      return mgr;
    }
  }

  typedef ASensorManager *(*PF_GETINSTANCE)();
  PF_GETINSTANCE getInstanceFunc = (PF_GETINSTANCE)
      dlsym(androidHandle, "ASensorManager_getInstance");
  // by all means at this point, ASensorManager_getInstance should be available
  assert(getInstanceFunc);
  dlclose(androidHandle);

  return getInstanceFunc();
}

}  // namespace ndkHelper
