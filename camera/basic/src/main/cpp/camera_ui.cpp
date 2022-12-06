
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
 */

#include <utils/native_debug.h>

#include "camera_engine.h"

/**
 * Retrieve current rotation from Java side
 *
 * @return current rotation angle
 */
int CameraEngine::GetDisplayRotation() {
  ASSERT(app_, "Application is not initialized");

  JNIEnv *env;
  ANativeActivity *activity = app_->activity;
  activity->vm->GetEnv((void **)&env, JNI_VERSION_1_6);

  activity->vm->AttachCurrentThread(&env, NULL);

  jobject activityObj = env->NewGlobalRef(activity->clazz);
  jclass clz = env->GetObjectClass(activityObj);
  jint newOrientation = env->CallIntMethod(
      activityObj, env->GetMethodID(clz, "getRotationDegree", "()I"));
  env->DeleteGlobalRef(activityObj);

  activity->vm->DetachCurrentThread();
  return newOrientation;
}

/**
 * Initializate UI on Java side. The 2 seekBars' values are passed in
 * array in the tuple of ( min, max, curVal )
 *   0: exposure min
 *   1: exposure max
 *   2: exposure val
 *   3: sensitivity min
 *   4: sensitivity max
 *   5: sensitivity val
 */
const int kInitDataLen = 6;
void CameraEngine::EnableUI(void) {
  JNIEnv *jni;
  app_->activity->vm->AttachCurrentThread(&jni, NULL);
  int64_t range[3];

  // Default class retrieval
  jclass clazz = jni->GetObjectClass(app_->activity->clazz);
  jmethodID methodID = jni->GetMethodID(clazz, "EnableUI", "([J)V");
  jlongArray initData = jni->NewLongArray(kInitDataLen);

  ASSERT(initData && methodID, "JavaUI interface Object failed(%p, %p)",
         methodID, initData);

  if (!camera_->GetExposureRange(&range[0], &range[1], &range[2])) {
    memset(range, 0, sizeof(int64_t) * 3);
  }

  jni->SetLongArrayRegion(initData, 0, 3, range);

  if (!camera_->GetSensitivityRange(&range[0], &range[1], &range[2])) {
    memset(range, 0, sizeof(int64_t) * 3);
  }
  jni->SetLongArrayRegion(initData, 3, 3, range);
  jni->CallVoidMethod(app_->activity->clazz, methodID, initData);
  app_->activity->vm->DetachCurrentThread();
}

/**
 * Handles UI request to take a photo into
 *   /sdcard/DCIM/Camera
 */
void CameraEngine::OnTakePhoto() {
  if (camera_) {
    camera_->TakePhoto();
  }
}

void CameraEngine::OnPhotoTaken(const char *fileName) {
  JNIEnv *jni;
  app_->activity->vm->AttachCurrentThread(&jni, NULL);

  // Default class retrieval
  jclass clazz = jni->GetObjectClass(app_->activity->clazz);
  jmethodID methodID =
      jni->GetMethodID(clazz, "OnPhotoTaken", "(Ljava/lang/String;)V");
  jstring javaName = jni->NewStringUTF(fileName);

  jni->CallVoidMethod(app_->activity->clazz, methodID, javaName);
  app_->activity->vm->DetachCurrentThread();
}
/**
 * Process user camera and disk writing permission
 * Resume application initialization after user granted camera and disk usage
 * If user denied permission, do nothing: no camera
 *
 * @param granted user's authorization for camera and disk usage.
 * @return none
 */
void CameraEngine::OnCameraPermission(jboolean granted) {
  cameraGranted_ = (granted != JNI_FALSE);

  if (cameraGranted_) {
    OnAppInitWindow();
  }
}

/**
 *  A couple UI handles ( from UI )
 *      user camera and disk permission
 *      exposure and sensitivity SeekBars
 *      takePhoto button
 */
extern "C" JNIEXPORT void JNICALL
Java_com_sample_camera_basic_CameraActivity_notifyCameraPermission(
    JNIEnv *env, jclass type, jboolean permission) {
  std::thread permissionHandler(&CameraEngine::OnCameraPermission,
                                GetAppEngine(), permission);
  permissionHandler.detach();
}

extern "C" JNIEXPORT void JNICALL
Java_com_sample_camera_basic_CameraActivity_TakePhoto(JNIEnv *env,
                                                      jclass type) {
  std::thread takePhotoHandler(&CameraEngine::OnTakePhoto, GetAppEngine());
  takePhotoHandler.detach();
}

extern "C" JNIEXPORT void JNICALL
Java_com_sample_camera_basic_CameraActivity_OnExposureChanged(
    JNIEnv *env, jobject instance, jlong exposurePercent) {
  GetAppEngine()->OnCameraParameterChanged(ACAMERA_SENSOR_EXPOSURE_TIME,
                                           exposurePercent);
}

extern "C" JNIEXPORT void JNICALL
Java_com_sample_camera_basic_CameraActivity_OnSensitivityChanged(
    JNIEnv *env, jobject instance, jlong sensitivity) {
  GetAppEngine()->OnCameraParameterChanged(ACAMERA_SENSOR_SENSITIVITY,
                                           sensitivity);
}
