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

#include "camera_manager.h"
#include "utils/native_debug.h"
#include "image_reader.h"


/*
 * Very simple application stub to manage camera and display for Preview
 */
class Engine {
  public:
    explicit Engine(android_app* app) :
            app_(app),
            reader_(nullptr),
            androidCamera_(nullptr),
            animationReady(false) {readerReady = false;}
    ~Engine() {
        DeleteCamera();
    }

    struct android_app* AndroidApp(void) const { return app_; }
    void RequestCameraPermission();
    void UpdateCameraPermission(jboolean granted);
    bool isCameraGranted(void) { return cameraGranted_; }
    void CreateCamera(void);
    void DeleteCamera(void);
    void OnDoubleTap(void);
    void DrawFrame(void);

    volatile bool animationReady;  // experiment, delete later
    volatile bool readerReady;
    void OnAppConfigChange(void);
    void OnAppInitWindow(void);
    void OnAppTermWindow(void);
  private:
    int rotation_;
    int GetDisplayRotation(void);
    struct android_app* app_;
    bool cameraGranted_;
    NativeCamera * androidCamera_;
    ImageReader* reader_;
};

/*
 * Handle APP_INIT_WINDOW message
 */
void Engine::OnAppInitWindow(void) {
    // NativeActivity end is ready to display, start pulling images
    //   if necessary
    animationReady = true;
    rotation_ = GetDisplayRotation();

    if (!cameraGranted_) {
        // Not permitted to use camera yet, ask(again) and defer other events
        RequestCameraPermission();
        return;
    }

    CreateCamera();
    OnDoubleTap();  // Start preview
}

/*
 * Handle APP_CMD_TEMR_WINDOW
 */
void Engine::OnAppTermWindow(void) {
    DeleteCamera();
    animationReady = false;
}
/*
 * Handle APP_CMD_CONFIG_CHANGED
 */
void Engine::OnAppConfigChange(void) {
    int newRotation = GetDisplayRotation();
    LOGI("=====New Orientation: %d", newRotation);

    if (newRotation != rotation_) {
        OnAppTermWindow();
        OnAppInitWindow();
    }
}
void Engine::UpdateCameraPermission(jboolean granted) {
    cameraGranted_ = (granted != 0);
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

    int32_t displayRotation = GetDisplayRotation();
    ASSERT(displayRotation == rotation_, "display rotated");
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

    ImageResolution res {0, 0, 0};
    androidCamera_->FindSupportedCaptureResolution(app_->window, &res);

    ASSERT(res.width && res.height, "Could not find supportable resolution");

    reader_= new ImageReader(&res);
    reader_->SetPresentRotation(imageRotation);
    readerReady = true;

    // now we could create session
    androidCamera_->CreateSession(reader_->GetNativeWindow());
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
    if (!readerReady)
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
    static int32_t format = WINDOW_FORMAT_RGBX_8888;
    Engine* engine = reinterpret_cast<Engine*>(app->userData);
    switch (cmd) {
        case APP_CMD_INIT_WINDOW:
            if (engine->AndroidApp()->window != NULL) {
                format = ANativeWindow_getFormat(app->window);
                ANativeWindow_setBuffersGeometry(app->window,
                              ANativeWindow_getWidth(app->window),
                              ANativeWindow_getHeight(app->window),
                              WINDOW_FORMAT_RGBX_8888);
                engine->OnAppInitWindow();
            }
            break;
        case APP_CMD_TERM_WINDOW:
            engine->OnAppTermWindow();
            ANativeWindow_setBuffersGeometry(app->window,
                          ANativeWindow_getWidth(app->window),
                          ANativeWindow_getHeight(app->window),
                          format);
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

    // Make sure glue isn't stripped.
    app_dummy();

    state->userData = reinterpret_cast<void*>(&engine);
    state->onAppCmd = ProcessAndroidCmd;
    state->onInputEvent = ProcessAndroidInput;

    // loop waiting for stuff to do.
    while (1) {
        // Read all pending events.
        int events;
        struct android_poll_source* source;

        while (ALooper_pollAll(engine.animationReady ? 0 : -1, NULL, &events, (void**)&source) >= 0) {

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
        if (engine.animationReady) {
            pEngineObj->DrawFrame();
        }
    }
}

/*
 * notifyCameraPermission():
 *     handle Camera permission result. If granted, kick off another thread (
 *     so we are off from main UI thread ) to create a camera manager object
 *     and start preview; otherwise, just record result into engine, and
 *     proactively asking next time (min/max application event).
 *     We are proactive because we have to have camera permission to run the
 *     sample.
 */
void* CreateCamera(void *ctx) {
    reinterpret_cast<Engine*>(ctx)->CreateCamera();
    reinterpret_cast<Engine*>(ctx)->OnDoubleTap();
    return nullptr;
}
extern "C" JNIEXPORT void JNICALL
Java_com_sample_camera_readimage_ReadImageActivity_notifyCameraPermission(JNIEnv *env, jclass type,
                                                                jboolean permission) {
    ASSERT(pEngineObj, "NativeEngine is NOT initialized");
    pEngineObj->UpdateCameraPermission(permission);
    if (permission) {
        pthread_t notifyThread;
        int error = pthread_create(&notifyThread, nullptr, ::CreateCamera, pEngineObj);
        ASSERT(error == 0, "Create Starting Camera thread failed");
    }
}
