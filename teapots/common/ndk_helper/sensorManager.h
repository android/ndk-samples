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

//--------------------------------------------------------------------------------
// sensorManager.h
//--------------------------------------------------------------------------------
#ifndef SENSORMANAGER_H_
#define SENSORMANAGER_H_

#include <android/sensor.h>
#include "JNIHelper.h"

namespace ndk_helper {
//--------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------
enum ORIENTATION {
  ORIENTATION_UNKNOWN = -1,
  ORIENTATION_PORTRAIT = 0,
  ORIENTATION_LANDSCAPE = 1,
  ORIENTATION_REVERSE_PORTRAIT = 2,
  ORIENTATION_REVERSE_LANDSCAPE = 3,
};

/*
 * Helper to handle sensor inputs such as accelerometer.
 * The helper also check for screen rotation
 *
 */
class SensorManager {
  ASensorManager *sensorManager_;
  const ASensor *accelerometerSensor_;
  ASensorEventQueue *sensorEventQueue_;

 protected:
 public:
  SensorManager();
  ~SensorManager();
  void Init(android_app *state);
  void Suspend();
  void Resume();
};

/*
 * AcquireASensorManagerInstance(android_app* app)
 *    Workaround ASensorManager_getInstance() deprecation false alarm
 *    for Android-N and before, when compiling with NDK-r15
 */
ASensorManager* AcquireASensorManagerInstance(android_app* app);
}  // namespace ndkHelper
#endif /* SENSORMANAGER_H_ */
