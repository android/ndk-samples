/*
 * Copyright 2016 The Android Open Source Project
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
// Include files
//--------------------------------------------------------------------------------
#include <android/log.h>
#include <android_native_app_glue.h>
#include <condition_variable>
#include <dlfcn.h>
#include <EGL/egl.h>
#include <thread>

#include "TeapotRenderer.h"
#include "NDKHelper.h"
//-------------------------------------------------------------------------
// Preprocessor
//-------------------------------------------------------------------------
#define HELPER_CLASS_NAME \
  "com/sample/helper/NDKHelper"  // Class name of helper function

// Indicate API mode to achieve 30 FPS.
enum APIMode {
  kAPINone,
  kAPINativeChoreographer,
  kAPIJavaChoreographer,
  kAPIEGLExtension,
};

const int32_t kFPSThrottleInterval = 2;
const int64_t kFPSThrottlePresentationInterval = (kFPSThrottleInterval / 60.0f) * 1000000000;
#define COULD_RENDER(now, last) (((now) - (last)) >= kFPSThrottlePresentationInterval)

// Declaration for native chreographer API.
struct AChoreographer;
typedef void (*AChoreographer_frameCallback)(long frameTimeNanos, void* data);
typedef AChoreographer* (*func_AChoreographer_getInstance)();
typedef void (*func_AChoreographer_postFrameCallback)(
    AChoreographer* choreographer, AChoreographer_frameCallback callback,
    void* data);

//-------------------------------------------------------------------------
// Shared state for our app.
//-------------------------------------------------------------------------
struct android_app;
class Engine {
  android_app* app_;

  TeapotRenderer renderer_;

  ndk_helper::GLContext* gl_context_;

  bool initialized_resources_;
  bool has_focus_;
  bool fps_throttle_;

  ndk_helper::DoubletapDetector doubletap_detector_;
  ndk_helper::PinchDetector pinch_detector_;
  ndk_helper::DragDetector drag_detector_;
  ndk_helper::PerfMonitor monitor_;

  ndk_helper::TapCamera tap_camera_;

  APIMode api_mode_;
  APIMode original_api_mode_;

  void UpdateFPS(float fFPS);
  void ShowUI();
  void TransformPosition(ndk_helper::Vec2& vec);
  void Swap();

  // Do swap operation at the end of rendering if necessary.
  void DoSwap();
  void CheckAPISupport();
  void StartFPSThrottle();
  void StopFPSThrottle();
  int64_t GetCurrentTime();

  void StartChoreographer();
  void StartJavaChoreographer();
  void StopJavaChoreographer();
  static void choreographer_callback(long frameTimeNanos, void* data);

  // Function pointers for native Choreographer API.
  func_AChoreographer_getInstance AChoreographer_getInstance_;
  func_AChoreographer_postFrameCallback AChoreographer_postFrameCallback_;

  // Stuff for EGL Android presentation time extension.
  int64_t presentation_time_;
  bool (*eglPresentationTimeANDROID_)(EGLDisplay dpy, EGLSurface sur,
                                      khronos_stime_nanoseconds_t time);

  int64_t prevFrameTimeNanos_;
  bool should_render_;
  std::mutex mtx_;              // mutex for critical section
  std::condition_variable cv_;  // condition variable for critical section

 public:
  static void HandleCmd(struct android_app* app, int32_t cmd);
  static int32_t HandleInput(android_app* app, AInputEvent* event);

  Engine();
  ~Engine();
  void SetState(android_app *app);
  int InitDisplay(android_app *app);
  void LoadResources();
  void UnloadResources();
  void DrawFrame();
  void TermDisplay();
  void TrimMemory();
  bool IsReady();
  void UpdatePosition(AInputEvent* event, int32_t iIndex, float& fX, float& fY);

  // Do swap operation while Choreographer callback. Need to be a public method
  // since it's called from JNI callback.
  void SynchInCallback(jlong frameTimeNamos);
};

// Global instance of the Engine class.
Engine g_engine;

Engine::Engine()
    :app_(NULL),
     initialized_resources_(false),
     has_focus_(false),
     fps_throttle_(true),
     api_mode_(kAPINone),
     prevFrameTimeNanos_(static_cast<int64_t>(0)),
     should_render_(true) {
  gl_context_ = ndk_helper::GLContext::GetInstance();
}

Engine::~Engine() {}

/**
 * Check which API is supported in the device.
 */
void Engine::CheckAPISupport() {
  auto apilevel = AConfiguration_getSdkVersion(app_->config);
  LOGI("Device API Level %d", apilevel);

  if (apilevel >= 24) {
    // Native Choreographer API is supported in API level 24~.
    void* lib = dlopen("libandroid.so", RTLD_NOW | RTLD_LOCAL);
    if (lib != nullptr) {
      LOGI("Run with Choreographer Native API.");
      api_mode_ = kAPINativeChoreographer;

      // Retrieve function pointers from shared object.
      AChoreographer_getInstance_ =
          reinterpret_cast<func_AChoreographer_getInstance>(
              dlsym(lib, "AChoreographer_getInstance"));
      AChoreographer_postFrameCallback_ =
          reinterpret_cast<func_AChoreographer_postFrameCallback>(
              dlsym(lib, "AChoreographer_postFrameCallback"));
      assert(AChoreographer_getInstance_);
      assert(AChoreographer_postFrameCallback_);
    }
  } else if (apilevel >= 18) {
    // eglPresentationTimeANDROID would be supported in API level 18~.
    LOGI("Run with EGLExtension.");
    api_mode_ = kAPIEGLExtension;

    // Retrieve the EGL extension's function pointer.
    eglPresentationTimeANDROID_ = reinterpret_cast<
        bool (*)(EGLDisplay, EGLSurface, khronos_stime_nanoseconds_t)>(
        eglGetProcAddress("eglPresentationTimeANDROID"));
    assert(eglPresentationTimeANDROID_);
    presentation_time_ = GetCurrentTime();
  } else if (apilevel >= 16) {
    // Choreographer Java API is supported API level 16~.
    LOGI("Run with Chreographer Java API.");
    api_mode_ = kAPIJavaChoreographer;
  } else {
    api_mode_ = kAPINone;
  }
  original_api_mode_ = api_mode_;
  StartFPSThrottle();
}

void Engine::StartFPSThrottle() {
  api_mode_ = original_api_mode_;
  if (api_mode_ == kAPINativeChoreographer) {
    // Initiate choreographer callback.
    StartChoreographer();
  } else if (api_mode_ == kAPIJavaChoreographer) {
    // Initiate Java choreographer callback.
    StartJavaChoreographer();
  }
}

void Engine::StopFPSThrottle() {
  if (api_mode_ == kAPINativeChoreographer) {
    should_render_ = true;
    //    ALooper_wake(app_->looper);
  } else if (api_mode_ == kAPIJavaChoreographer) {
    StopJavaChoreographer();
  }
  api_mode_ = kAPINone;
}

void Engine::DoSwap() {
  if (api_mode_ == kAPINativeChoreographer) {
    // Use choreographer to synchronize.
    // Do nothing but wait the until choreographer callback.
    should_render_ = false;
  } else if (api_mode_ == kAPIJavaChoreographer) {
    // Wait until the conditional variable is signaled.
    std::unique_lock<std::mutex> lock(mtx_);
    cv_.wait(lock);
    Swap();
  } else if (api_mode_ == kAPIEGLExtension) {
    // Use eglPresentationTimeANDROID extension.
    presentation_time_ += kFPSThrottlePresentationInterval;
    eglPresentationTimeANDROID_(gl_context_->GetDisplay(),
                                gl_context_->GetSurface(), presentation_time_);
    Swap();
  } else {
    // Regular Swap.
    Swap();
  }
}

// Native Chreographer API support.
void Engine::StartChoreographer() {
  // Initiate choreographer callbacks.
  if (api_mode_ == kAPINativeChoreographer) {
    auto choreographer = AChoreographer_getInstance_();
    AChoreographer_postFrameCallback_(choreographer, choreographer_callback,
                                      this);
  }
}

// Native Choreographer callback.
void Engine::choreographer_callback(long frameTimeNanos, void* data) {
  auto engine = reinterpret_cast<Engine*>(data);

  // Post next callback for self.
  if (engine->has_focus_) {
    engine->StartChoreographer();
  }

  // Swap buffer if the timing meets the 30fps time interval condition.
  // The callback is in the same thread context, so that we can just invoke
  // eglSwapBuffers().
  if (COULD_RENDER(frameTimeNanos, engine->prevFrameTimeNanos_)) {
    engine->should_render_ = true;
    engine->Swap();
    // Wake up main looper so that it will continue rendering.
    ALooper_wake(engine->app_->looper);
    engine->prevFrameTimeNanos_ = frameTimeNanos;
  }
}

// Java choreographer API support.
// With Java API, we uses synch primitive to synchronize Java thread and render
// thread.
void Engine::StartJavaChoreographer() {
  JNIEnv* jni;
  app_->activity->vm->AttachCurrentThread(&jni, NULL);
  // Intiate Java Chreographer API.
  jclass clazz = jni->GetObjectClass(app_->activity->clazz);
  jmethodID methodID = jni->GetMethodID(clazz, "startChoreographer", "()V");
  jni->CallVoidMethod(app_->activity->clazz, methodID);
  app_->activity->vm->DetachCurrentThread();
  return;
}

void Engine::StopJavaChoreographer() {
  JNIEnv* jni;
  app_->activity->vm->AttachCurrentThread(&jni, NULL);
  // Intiate Java Chreographer API.
  jclass clazz = jni->GetObjectClass(app_->activity->clazz);
  jmethodID methodID = jni->GetMethodID(clazz, "stopChoreographer", "()V");
  jni->CallVoidMethod(app_->activity->clazz, methodID);
  app_->activity->vm->DetachCurrentThread();
  // Make sure the render thread is not blocked.
  cv_.notify_one();
  return;
}

void Engine::SynchInCallback(jlong frameTimeInNanos) {
  // Signal render thread if the timing meets the 30fps time interval condition.
  if (COULD_RENDER(frameTimeInNanos, prevFrameTimeNanos_)) {
    prevFrameTimeNanos_ = frameTimeInNanos;
    cv_.notify_one();
  }
};

extern "C" JNIEXPORT void JNICALL
Java_com_sample_choreographer_ChoreographerNativeActivity_choregrapherCallback(
    JNIEnv* env, jobject instance, jlong frameTimeInNanos) {
  g_engine.SynchInCallback(frameTimeInNanos);
}

// Helper functions.
int64_t Engine::GetCurrentTime() {
  timespec time;
  clock_gettime(CLOCK_MONOTONIC, &time);
  return static_cast<int64_t>(time.tv_sec) * 1e9 +
         static_cast<int64_t>(time.tv_nsec);
}

void Engine::Swap() {
  if (EGL_SUCCESS != gl_context_->Swap()) {
    UnloadResources();
    LoadResources();
  }
}

/**
 * Load resources
 */
void Engine::LoadResources() {
  renderer_.Init();
  renderer_.Bind(&tap_camera_);
}

/**
 * Unload resources
 */
void Engine::UnloadResources() { renderer_.Unload(); }

/**
 * Initialize an EGL context for the current display.
 */
int Engine::InitDisplay(android_app *app) {
  if (!initialized_resources_) {
    gl_context_->Init(app_->window);
    LoadResources();
    initialized_resources_ = true;
  } else if(app->window != gl_context_->GetANativeWindow()) {
    // Re-initialize ANativeWindow.
    // On some devices, ANativeWindow is re-created when the app is resumed
    assert(gl_context_->GetANativeWindow());
    UnloadResources();
    gl_context_->Invalidate();
    app_ = app;
    gl_context_->Init(app->window);
    LoadResources();
    initialized_resources_ = true;
  } else {
    // initialize OpenGL ES and EGL
    if (EGL_SUCCESS != gl_context_->Resume(app_->window)) {
      UnloadResources();
      LoadResources();
    }
  }

  ShowUI();

  // Initialize GL state.
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);

  // Note that screen size might have been changed
  glViewport(0, 0, gl_context_->GetScreenWidth(),
             gl_context_->GetScreenHeight());
  renderer_.UpdateViewport();

  tap_camera_.SetFlip(1.f, -1.f, -1.f);
  tap_camera_.SetPinchTransformFactor(2.f, 2.f, 8.f);

  return 0;
}

/**
 * Just the current frame in the display.
 */
void Engine::DrawFrame() {
  float fps;
  if (monitor_.Update(fps)) {
    UpdateFPS(fps);
  }
  renderer_.Update(monitor_.GetCurrentTime());

  // Just fill the screen with a color.
  glClearColor(0.5f, 0.5f, 0.5f, 1.f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  float color[2][3] = {{1.0f, 0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}};
  int32_t i = fps_throttle_ ? 0 : 1;
  renderer_.Render(color[i][0], color[i][1], color[i][2]);
  DoSwap();
}

/**
 * Tear down the EGL context currently associated with the display.
 */
void Engine::TermDisplay() { gl_context_->Suspend(); }

void Engine::TrimMemory() {
  LOGI("Trimming memory");
  gl_context_->Invalidate();
}

/**
 * Process the next input event.
 */
int32_t Engine::HandleInput(android_app* app, AInputEvent* event) {
  Engine* eng = (Engine*)app->userData;
  if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
    ndk_helper::GESTURE_STATE doubleTapState =
        eng->doubletap_detector_.Detect(event);
    ndk_helper::GESTURE_STATE dragState = eng->drag_detector_.Detect(event);
    ndk_helper::GESTURE_STATE pinchState = eng->pinch_detector_.Detect(event);

    // Double tap detector has a priority over other detectors
    if (doubleTapState == ndk_helper::GESTURE_STATE_ACTION) {
      // Detect double tap
      eng->tap_camera_.Reset(true);

      // Switch mode between 30FPS <-> 60FPS.
      eng->fps_throttle_ = !eng->fps_throttle_;
      LOGI("Switched FPS throttle mode.");
      if (eng->fps_throttle_) {
        eng->StartFPSThrottle();
      } else {
        eng->StopFPSThrottle();
      }
    } else {
      // Handle drag state
      if (dragState & ndk_helper::GESTURE_STATE_START) {
        // Otherwise, start dragging
        ndk_helper::Vec2 v;
        eng->drag_detector_.GetPointer(v);
        eng->TransformPosition(v);
        eng->tap_camera_.BeginDrag(v);
      } else if (dragState & ndk_helper::GESTURE_STATE_MOVE) {
        ndk_helper::Vec2 v;
        eng->drag_detector_.GetPointer(v);
        eng->TransformPosition(v);
        eng->tap_camera_.Drag(v);
      } else if (dragState & ndk_helper::GESTURE_STATE_END) {
        eng->tap_camera_.EndDrag();
      }

      // Handle pinch state
      if (pinchState & ndk_helper::GESTURE_STATE_START) {
        // Start new pinch
        ndk_helper::Vec2 v1;
        ndk_helper::Vec2 v2;
        eng->pinch_detector_.GetPointers(v1, v2);
        eng->TransformPosition(v1);
        eng->TransformPosition(v2);
        eng->tap_camera_.BeginPinch(v1, v2);
      } else if (pinchState & ndk_helper::GESTURE_STATE_MOVE) {
        // Multi touch
        // Start new pinch
        ndk_helper::Vec2 v1;
        ndk_helper::Vec2 v2;
        eng->pinch_detector_.GetPointers(v1, v2);
        eng->TransformPosition(v1);
        eng->TransformPosition(v2);
        eng->tap_camera_.Pinch(v1, v2);
      }
    }
    return 1;
  }
  return 0;
}

/**
 * Process the next main command.
 */
void Engine::HandleCmd(struct android_app* app, int32_t cmd) {
  Engine* eng = (Engine*)app->userData;
  switch (cmd) {
    case APP_CMD_SAVE_STATE:
      break;
    case APP_CMD_INIT_WINDOW:
      // The window is being shown, get it ready.
      if (app->window != NULL) {
        eng->InitDisplay(app);
        eng->has_focus_ = true;
        eng->DrawFrame();
      }
      break;
    case APP_CMD_TERM_WINDOW:
      // The window is being hidden or closed, clean it up.
      eng->TermDisplay();
      eng->has_focus_ = false;
      break;
    case APP_CMD_STOP:
      break;
    case APP_CMD_GAINED_FOCUS:
      // Start animation
      eng->has_focus_ = true;

      // Update counter when the app becomes active.
      eng->presentation_time_ = eng->GetCurrentTime();
      if (eng->api_mode_ == kAPINativeChoreographer) {
        eng->StartChoreographer();
      }
      break;
    case APP_CMD_LOST_FOCUS:
      // Also stop animating.
      eng->has_focus_ = false;
      eng->DrawFrame();
      break;
    case APP_CMD_LOW_MEMORY:
      // Free up GL resources
      eng->TrimMemory();
      break;
  }
}

//-------------------------------------------------------------------------
// Misc
//-------------------------------------------------------------------------
void Engine::SetState(android_app* state) {
  app_ = state;
  doubletap_detector_.SetConfiguration(app_->config);
  drag_detector_.SetConfiguration(app_->config);
  pinch_detector_.SetConfiguration(app_->config);

  CheckAPISupport();
}

bool Engine::IsReady() {
  if (has_focus_ && should_render_) return true;

  return false;
}

void Engine::TransformPosition(ndk_helper::Vec2& vec) {
  vec = ndk_helper::Vec2(2.0f, 2.0f) * vec /
            ndk_helper::Vec2(gl_context_->GetScreenWidth(),
                             gl_context_->GetScreenHeight()) -
        ndk_helper::Vec2(1.f, 1.f);
}

void Engine::ShowUI() {
  JNIEnv* jni;
  app_->activity->vm->AttachCurrentThread(&jni, NULL);

  // Default class retrieval
  jclass clazz = jni->GetObjectClass(app_->activity->clazz);
  jmethodID methodID = jni->GetMethodID(clazz, "showUI", "()V");
  jni->CallVoidMethod(app_->activity->clazz, methodID);

  app_->activity->vm->DetachCurrentThread();
  return;
}

void Engine::UpdateFPS(float fFPS) {
  JNIEnv* jni;
  app_->activity->vm->AttachCurrentThread(&jni, NULL);

  // Default class retrieval
  jclass clazz = jni->GetObjectClass(app_->activity->clazz);
  jmethodID methodID = jni->GetMethodID(clazz, "updateFPS", "(F)V");
  jni->CallVoidMethod(app_->activity->clazz, methodID, fFPS);

  app_->activity->vm->DetachCurrentThread();
  return;
}

/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 */
void android_main(android_app* state) {

  g_engine.SetState(state);

  // Init helper functions
  ndk_helper::JNIHelper::Init(state->activity, HELPER_CLASS_NAME);

  state->userData = &g_engine;
  state->onAppCmd = Engine::HandleCmd;
  state->onInputEvent = Engine::HandleInput;

  // loop waiting for stuff to do.
  while (1) {
    // Read all pending events.
    int id;
    int events;
    android_poll_source* source;

    // If not animating, we will block forever waiting for events.
    // If animating, we loop until all events are read, then continue
    // to draw the next frame of animation.
    while ((id = ALooper_pollAll(g_engine.IsReady() ? 0 : -1, NULL, &events,
                                 (void**)&source)) >= 0) {
      // Process this event.
      if (source != NULL) source->process(state, source);

      // Check if we are exiting.
      if (state->destroyRequested != 0) {
        g_engine.TermDisplay();
        return;
      }
    }

    if (g_engine.IsReady()) {
      // Drawing is throttled to the screen update rate, so there
      // is no need to do timing here.
      g_engine.DrawFrame();
    }
  }
}