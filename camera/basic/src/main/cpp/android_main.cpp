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
#include <cstdio>
#include <android/native_window.h>
#include <android_native_app_glue.h>
#include <functional>
#include <thread>

#include "camera_manager.h"
#include "utils/native_debug.h"


/*
 * Simple Preview Engine
 */
class Engine {
  public:
    explicit Engine(android_app* app) :
            app_(app),
            reader_(nullptr),
            jpgReader_(nullptr),
            androidCamera_(nullptr),
            animationReady_(false),
            cameraGranted_(false),
            rotation_ (0) {
        memset( &orgNativeWinRes_, 0, sizeof(orgNativeWinRes_));
    }
    ~Engine() {
        DeleteCamera();
    }

    struct android_app* AndroidApp(void) const { return app_; }
    void RequestCameraPermission();
    void OnCameraPermission(jboolean granted);
    void CreateCamera(void);
    void DeleteCamera(void);
    void OnDoubleTap(void);
    void OnTakePhoto(void);
    void DrawFrame(void);

    volatile bool animationReady_;
    void OnAppConfigChange(void);
    void OnAppInitWindow(void);
    void OnAppTermWindow(void);

    int32_t GetSavedNativeWinWidth(void) {
        return orgNativeWinRes_.width;
    }
    int32_t GetSavedNativeWinHeight(void) {
        return orgNativeWinRes_.height;
    }
    int32_t GetSavedNativeWinFormat(void) {
        return orgNativeWinRes_.format;
    }
    void SaveNativeWinRes(int32_t w, int32_t h, int32_t format) {
        orgNativeWinRes_.width = w;
        orgNativeWinRes_.height = h;
        orgNativeWinRes_.format = format;
    }
  private:
    void EnableUI(void);

    int rotation_;
    int GetDisplayRotation(void);
    struct android_app* app_;
    bool cameraGranted_;
    NativeCamera * androidCamera_;
    ImageReader* reader_;
    ImageReader* jpgReader_;
    ImageResolution orgNativeWinRes_;
};

/*
 * Handle APP_INIT_WINDOW message
 */
void Engine::OnAppInitWindow(void) {
    if (!cameraGranted_) {
        // Not permitted to use camera yet, ask(again) and defer other events
        RequestCameraPermission();
        return;
    }

    // NativeActivity end is ready to display, start pulling images
    //   if necessary
    animationReady_ = true;
    rotation_ = GetDisplayRotation();

    CreateCamera();
    OnDoubleTap();  // Start preview
}

/*
 * Handle APP_CMD_TEMR_WINDOW
 */
void Engine::OnAppTermWindow(void) {
    DeleteCamera();
    animationReady_ = false;
}
/*
 * Handle APP_CMD_CONFIG_CHANGED
 */
void Engine::OnAppConfigChange(void) {
    int newRotation = GetDisplayRotation();
    LOGI("=====New Orientation: %d", newRotation);

    if (newRotation != rotation_) {
        OnAppTermWindow();

        rotation_ = newRotation;
        OnAppInitWindow();
    }
}
void Engine::OnCameraPermission(jboolean granted) {
    cameraGranted_ = (granted != JNI_FALSE);

    if(cameraGranted_) {
       OnAppInitWindow();
    }
}

/*
 * Create a camera object for onboard BACK_FACING camera
 */
void Engine::CreateCamera(void) {
    // Camera needed to be requested at the run-time from Java SDK
    // if Not-granted, do nothing.
    if (!cameraGranted_ || !app_->window) {
        LOGW("Camera Sample requires Full Camera access");
        return;
    }
    EnableUI();

    int32_t displayRotation = GetDisplayRotation();
    ASSERT(displayRotation == rotation_, "display rotated");
    rotation_ = displayRotation;
    ASSERT(androidCamera_== nullptr, "CameraObject is already initialized");

    androidCamera_ = new NativeCamera(nullptr);
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
    ImageResolution view {0, 0, 0}, capture {0, 0, 0};
    androidCamera_->FindSupportedCaptureResolution(app_->window, &view, &capture);

    ASSERT(view.width && view.height, "Could not find supportable resolution");

    // Request the necessary nativeWindow to OS
    bool portraitNativeWindow = (orgNativeWinRes_.width < orgNativeWinRes_.height);
    ANativeWindow_setBuffersGeometry(app_->window,
                                     portraitNativeWindow? view.height : view.width,
                                     portraitNativeWindow? view.width : view.height,
                                     WINDOW_FORMAT_RGBX_8888);

    reader_= new ImageReader(&view, AIMAGE_FORMAT_YUV_420_888);
    reader_->SetPresentRotation(imageRotation);
    jpgReader_ = new ImageReader(&capture, AIMAGE_FORMAT_JPEG);
    jpgReader_->SetPresentRotation(imageRotation);

    // now we could create session
    androidCamera_->CreateSession(reader_->GetNativeWindow(),
                                  jpgReader_->GetNativeWindow(),
                                  imageRotation);

}

void Engine::DeleteCamera(void) {
    if (androidCamera_) {
        delete androidCamera_;
        androidCamera_ = nullptr;
    }
    if (reader_) {
        delete reader_;
        reader_ = nullptr;
    }
    if (jpgReader_) {
      delete jpgReader_;
      jpgReader_ = nullptr;
    }

}

/*
 * Process Double Tap event from user:
 *      toggle the preview state ( start/pause )
 * The Tap event is defined by this app. Refer to ProcessAndroidInput()
 */
void Engine::OnDoubleTap(void) {
    if (androidCamera_) {
        androidCamera_->Animate();
    }
}

void Engine::OnTakePhoto() {
  if(androidCamera_) {
    androidCamera_->TakePhoto();
  }
}
/*
 * Initiate a Camera Run-time usage request to Java side implementation
 *  [ The request result will be passed back to us in function
 *    notifyCameraPermission()]
 */
void Engine::RequestCameraPermission() {
    if (!app_) return;

    JNIEnv* env;
    ANativeActivity* activity = app_->activity;
    activity->vm->GetEnv((void**)&env, JNI_VERSION_1_6);

    activity->vm->AttachCurrentThread(&env, NULL);

    jobject activityObj = env->NewGlobalRef(activity->clazz);
    jclass clz = env->GetObjectClass(activityObj);
    env->CallVoidMethod(activityObj,
                        env->GetMethodID(clz, "RequestCamera", "()V"));
    env->DeleteGlobalRef(activityObj);

    activity->vm->DetachCurrentThread();
}

int Engine::GetDisplayRotation() {
    ASSERT(app_, "Application is not initialized");

    JNIEnv* env;
    ANativeActivity* activity = app_->activity;
    activity->vm->GetEnv((void**)&env, JNI_VERSION_1_6);

    activity->vm->AttachCurrentThread(&env, NULL);

    jobject activityObj = env->NewGlobalRef(activity->clazz);
    jclass clz = env->GetObjectClass(activityObj);
    jint newOrientation = env->CallIntMethod(activityObj,
                        env->GetMethodID(clz, "getRotationDegree", "()I"));
    env->DeleteGlobalRef(activityObj);

    activity->vm->DetachCurrentThread();
    return newOrientation;
}

void Engine::DrawFrame(void) {
    if (!reader_ || !reader_->IsReady())
        return;

    ANativeWindow_acquire(app_->window);
    ANativeWindow_Buffer buf;
    if (ANativeWindow_lock(app_->window, &buf, nullptr) < 0) {
        return;
    }

    reader_->DisplayImage(&buf);
    ANativeWindow_unlockAndPost(app_->window);
    ANativeWindow_release(app_->window);
}

void Engine::EnableUI(void) {
  JNIEnv* jni;
  app_->activity->vm->AttachCurrentThread(&jni, NULL);

  // Default class retrieval
  jclass clazz = jni->GetObjectClass(app_->activity->clazz);
  jmethodID methodID = jni->GetMethodID(clazz, "EnableUI", "()V");
  jni->CallVoidMethod(app_->activity->clazz, methodID);

  app_->activity->vm->DetachCurrentThread();
}
/*
 * The stub engine reference:
 *     need by run-time camera request callback function
 */
Engine *pEngineObj = nullptr;

/*
 * ProcessAndroidInput:
 *   Capture the this-app-defined Tap Event:
 *     Tap:        touch down and up within less than half a second
 *     Double-Tap: 2 taps within 1 second
 */
#define TAP_THRESHOLD 500000000
static int32_t ProcessAndroidInput(struct android_app *app,
                                   AInputEvent *event) {
    static int64_t previousTapTime = static_cast<uint64_t>(0);

    Engine* engine = reinterpret_cast<Engine*>(app->userData);
    if (AInputEvent_getType(event) != AINPUT_EVENT_TYPE_MOTION    ||
        AInputEvent_getSource(event) != AINPUT_SOURCE_TOUCHSCREEN ||
        (AKeyEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK) != 
                AMOTION_EVENT_ACTION_UP) {
        return 0;
    }

    int64_t upTime = AMotionEvent_getEventTime(event);
    if ((upTime - AMotionEvent_getDownTime(event)) > TAP_THRESHOLD)
        return 1;

    // This tap and last tap event need to be within OUR defined time period
    // (1 sec). If too long apart, ignore the previous tap event
    if ((upTime - previousTapTime) > 2 * TAP_THRESHOLD) {
        previousTapTime = upTime;
        return 1;
    }
    
    // detected a double tap
    previousTapTime = static_cast<uint64_t>(0);
    engine->OnDoubleTap();

    return 1;
}
/*
 * ProcessAndroidCmd()
 *   Android command handler. Create/Delete camera object with
 *   INIT_WINDOW/TERM_WINDOW command, ignoring other event.
 */
static void ProcessAndroidCmd(struct android_app *app, int32_t cmd) {
    Engine* engine = reinterpret_cast<Engine*>(app->userData);
    switch (cmd) {
        case APP_CMD_INIT_WINDOW:
            if (engine->AndroidApp()->window != NULL) {
                engine->SaveNativeWinRes(ANativeWindow_getWidth(app->window),
                                         ANativeWindow_getHeight(app->window),
                                         ANativeWindow_getFormat(app->window));
                engine->OnAppInitWindow();
            }
            break;
        case APP_CMD_TERM_WINDOW:
            engine->OnAppTermWindow();
            ANativeWindow_setBuffersGeometry(app->window,
                          engine->GetSavedNativeWinWidth(),
                          engine->GetSavedNativeWinHeight(),
                          engine->GetSavedNativeWinFormat());
            break;
        case APP_CMD_CONFIG_CHANGED:
            engine->OnAppConfigChange();
            break;
        case APP_CMD_LOST_FOCUS:
            break;
    }
}

// Android application glue entry function for us
extern "C" void android_main(struct android_app* state) {

    Engine engine(state);
    pEngineObj = &engine;

    state->userData = reinterpret_cast<void*>(&engine);
    state->onAppCmd = ProcessAndroidCmd;
    state->onInputEvent = ProcessAndroidInput;

    // loop waiting for stuff to do.
    while (1) {
        // Read all pending events.
        int events;
        struct android_poll_source* source;

        while (ALooper_pollAll(engine.animationReady_ ? 0 : -1, NULL, &events, (void**)&source) >= 0) {

            // Process this event.
            if (source != NULL) {
                source->process(state, source);
            }

            // Check if we are exiting.
            if (state->destroyRequested != 0) {
                LOGI("Engine thread destroy requested!");
                engine.DeleteCamera();
                return;
            }
        }
        if (engine.animationReady_) {
            pEngineObj->DrawFrame();
        }
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_sample_camera_basic_CameraActivity_notifyCameraPermission(JNIEnv *env, jclass type,
                                                                jboolean permission) {
    ASSERT(pEngineObj, "NativeEngine is NOT initialized");
    std::thread permissionHandler(&Engine::OnCameraPermission, pEngineObj, permission);
    permissionHandler.detach();
}

extern "C" JNIEXPORT void JNICALL
Java_com_sample_camera_basic_CameraActivity_TakePhoto(JNIEnv *env, jclass type) {
    std::thread takePhotoHandler(&Engine::OnTakePhoto, pEngineObj);
    takePhotoHandler.detach();
}
