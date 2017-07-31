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

/* Description
 *     Demonstrate NDKCamera's PREVIEW mode -- hooks camera directly into
 *     display.
 *     Control:
 *         Double-Tap on Android device's screen to toggle start/stop preview
 *     Tested:
 *         Google Pixel and Nexus 6 phones
 */
#include <jni.h>
#include <cstdio>
#include <android/native_activity.h>
#include <android/native_window_jni.h>
#include <utils/camera_manager.h>
#include <utils/native_debug.h>

/*
 * Simple Preivew Engine
 */
class Engine {
  public:
    explicit Engine(JNIEnv* env, jobject surface) :
            surface_(surface),
            androidCamera_(nullptr),
            env_(env),
            requestHeight_(0),
            requestWidth_(0),
            rotation_ (0) { env->GetJavaVM(&vm_); }
    explicit Engine(JNIEnv* env, jint w, jint h, jint rotation):
            surface_(nullptr),
            androidCamera_(nullptr),
            env_ (env),
            requestHeight_(h),
            requestWidth_(w),
            rotation_ (rotation) { env->GetJavaVM(&vm_);}

    ~Engine() { DeleteCamera(); }

    ANativeWindow* getNativeWin(void) {
      return ANativeWindow_fromSurface(env_, surface_);
    }

    void CreateCamera(void);
    void CreateCameraSession(jobject surface) {
      surface_ = surface;
      androidCamera_->CreateSession(getNativeWin());
    }

    void StartPreview(void) {
      if (androidCamera_) {
        androidCamera_->Animate();
      }
    }

    void DeleteCamera(void);
    void OnDoubleTap(void) {
      StartPreview();
    }
    const rRect& GetCompatibleCameraRes() const {
      return compatibleCameraRes_;
    }
private:
    JNIEnv* env_;
    JavaVM* vm_;
    int     rotation_;
    jobject surface_;
    NativeCamera * androidCamera_;
    int32_t requestWidth_;
    int32_t requestHeight_;
    rRect   compatibleCameraRes_;
};

/*
 * Create a camera object for onboard BACK_FACING camera
 */
void Engine::CreateCamera(void) {
  ASSERT(androidCamera_== nullptr, "CameraObject is already initialized");

  androidCamera_ = new NativeCamera();
  ASSERT(androidCamera_, "Failed to Create CameraObject");
  int32_t facing = 0, angle = 0, imageRotation = 0;
  if (androidCamera_->GetSensorOrientation(&facing, &angle)) {
    if (facing == ACAMERA_LENS_FACING_FRONT) {
      imageRotation = (angle + rotation_) % 360;
      imageRotation = (360 - imageRotation) % 360;
    } else {
      imageRotation = (angle - rotation_ + 360) % 360;
    }
  }
  LOGI("Phone Rotation: %d, Present Rotation Angle: %d",
       rotation_, imageRotation);
  rRect in = {
      .w = requestWidth_,
      .h = requestHeight_,
  };
  compatibleCameraRes_ = androidCamera_->GetCompatibleSize(in);

}

void Engine::DeleteCamera(void) {
    if (androidCamera_) {
        delete androidCamera_;
        androidCamera_ = nullptr;
    }
}


/*
 * The stub engine reference:
 *     need by run-time camera request callback function
 */
Engine *pEngineObj = nullptr;

/*
 * Interface functions to Java code
 */
extern "C" JNIEXPORT jlong JNICALL
Java_com_sample_textureview_ViewActivity_CreateCamera(
        JNIEnv *env,
        jobject instance,
        jint width,
        jint height,
        jint rotation) {

  pEngineObj = new Engine(env, width, height, rotation);
  pEngineObj->CreateCamera();

  return reinterpret_cast<jlong>(pEngineObj);
}

/*
 * Return Compatible preview camera resolution width
 *    Backfacing camera, the lowest resolution with aspection ration
 *    be same as display resolution
 */
extern "C" JNIEXPORT jint JNICALL
Java_com_sample_textureview_ViewActivity_GetCameraCompatibleWidth(
    JNIEnv *env,
    jobject instance) {

  return pEngineObj->GetCompatibleCameraRes().w;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_sample_textureview_ViewActivity_GetCameraCompatibleHeight(
    JNIEnv *env,
    jobject instance) {

  return pEngineObj->GetCompatibleCameraRes().h;
}

/*
 * Start Preview with given surface
 */
extern "C" JNIEXPORT void JNICALL
Java_com_sample_textureview_ViewActivity_notifySurfaceTextureCreated(
    JNIEnv *env, jobject instance, jobject surface) {
   pEngineObj->CreateCameraSession(surface);
   pEngineObj->StartPreview();
}

/*
 * Cleanup when surface is destroyed
 */
extern "C" JNIEXPORT void JNICALL
Java_com_sample_textureview_ViewActivity_notifySurfaceTextureDestroyed(
    JNIEnv *env, jobject instance, jobject surface) {
  delete pEngineObj;
  pEngineObj = nullptr;
}

