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

// BEGIN_INCLUDE(all)
#include <EGL/egl.h>
#include <GLES/gl.h>
#include <android/choreographer.h>
#include <android/log.h>
#include <android/sensor.h>
#include <android/set_abort_message.h>
#include <android_native_app_glue.h>
#include <jni.h>

#include <cassert>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <memory>

#define LOG_TAG "native-activity"

#define _LOG(priority, fmt, ...) \
  ((void)__android_log_print((priority), (LOG_TAG), (fmt)__VA_OPT__(, ) __VA_ARGS__))

#define LOGE(fmt, ...) _LOG(ANDROID_LOG_ERROR, (fmt)__VA_OPT__(, ) __VA_ARGS__)
#define LOGW(fmt, ...) _LOG(ANDROID_LOG_WARN, (fmt)__VA_OPT__(, ) __VA_ARGS__)
#define LOGI(fmt, ...) _LOG(ANDROID_LOG_INFO, (fmt)__VA_OPT__(, ) __VA_ARGS__)

[[noreturn]] __attribute__((__format__(__printf__, 1, 2))) static void fatal(
    const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  char* buf;
  if (vasprintf(&buf, fmt, ap) < 0) {
    android_set_abort_message("failed for format error message");
  } else {
    android_set_abort_message(buf);
    // Also log directly, since the default Android Studio logcat filter hides
    // the backtrace which would otherwise show the abort message.
    LOGE("%s", buf);
  }
  std::abort();
}

#define CHECK_NOT_NULL(value)                                           \
  do {                                                                  \
    if ((value) == nullptr) {                                           \
      fatal("%s:%d:%s must not be null", __PRETTY_FUNCTION__, __LINE__, \
            #value);                                                    \
    }                                                                   \
  } while (false)

/**
 * Our saved state data.
 */
struct SavedState {
  float angle;
  int32_t x;
  int32_t y;
};

/**
 * Shared state for our app.
 */
struct Engine {
  android_app* app;

  ASensorManager* sensorManager;
  const ASensor* accelerometerSensor;
  ASensorEventQueue* sensorEventQueue;

  EGLDisplay display;
  EGLSurface surface;
  EGLContext context;
  int32_t width;
  int32_t height;
  SavedState state;

  void CreateSensorListener(ALooper_callbackFunc callback) {
    CHECK_NOT_NULL(app);

    sensorManager = ASensorManager_getInstance();
    if (sensorManager == nullptr) {
      return;
    }

    accelerometerSensor = ASensorManager_getDefaultSensor(
        sensorManager, ASENSOR_TYPE_ACCELEROMETER);
    sensorEventQueue = ASensorManager_createEventQueue(
        sensorManager, app->looper, ALOOPER_POLL_CALLBACK, callback, this);
  }

  /// Resumes ticking the application.
  void Resume() {
    // Checked to make sure we don't double schedule Choreographer.
    if (!running_) {
      running_ = true;
      ScheduleNextTick();
    }
  }

  /// Pauses ticking the application.
  ///
  /// When paused, sensor and input events will still be processed, but the
  /// update and render parts of the loop will not run.
  void Pause() { running_ = false; }

 private:
  bool running_;

  void ScheduleNextTick() {
    AChoreographer_postFrameCallback(AChoreographer_getInstance(), Tick, this);
  }

  /// Entry point for Choreographer.
  ///
  /// The first argument (the frame time) is not used as it is not needed for
  /// this sample. If you copy from this sample and make use of that argument,
  /// note that there's an API bug: that time is a signed 32-bit nanosecond
  /// counter on 32-bit systems, so it will roll over every ~2 seconds. If your
  /// minSdkVersion is 29 or higher, use AChoreographer_postFrameCallback64
  /// instead, which is 64-bits for all architectures. Otherwise, bitwise-and
  /// the value with the upper bits from CLOCK_MONOTONIC.
  ///
  /// \param data The Engine being ticked.
  static void Tick(long, void* data) {
    CHECK_NOT_NULL(data);
    auto engine = reinterpret_cast<Engine*>(data);
    engine->DoTick();
  }

  void DoTick() {
    if (!running_) {
      return;
    }

    // Input and sensor feedback is handled via their own callbacks.
    // Choreographer ensures that those callbacks run before this callback does.

    // Choreographer does not continuously schedule the callback. We have to re-
    // register the callback each time we're ticked.
    ScheduleNextTick();
    Update();
    DrawFrame();
  }

  void Update() {
    state.angle += .01f;
    if (state.angle > 1) {
      state.angle = 0;
    }
  }

  void DrawFrame() {
    if (display == nullptr) {
      // No display.
      return;
    }

    // Just fill the screen with a color.
    glClearColor(((float)state.x) / width, state.angle,
                 ((float)state.y) / height, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    eglSwapBuffers(display, surface);
  }
};

/**
 * Initialize an EGL context for the current display.
 */
static int engine_init_display(Engine* engine) {
  // initialize OpenGL ES and EGL

  /*
   * Here specify the attributes of the desired configuration.
   * Below, we select an EGLConfig with at least 8 bits per color
   * component compatible with on-screen windows
   */
  const EGLint attribs[] = {EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                            EGL_BLUE_SIZE,    8,
                            EGL_GREEN_SIZE,   8,
                            EGL_RED_SIZE,     8,
                            EGL_NONE};
  EGLint w, h, format;
  EGLint numConfigs;
  EGLConfig config = nullptr;
  EGLSurface surface;
  EGLContext context;

  EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

  eglInitialize(display, nullptr, nullptr);

  /* Here, the application chooses the configuration it desires.
   * find the best match if possible, otherwise use the very first one
   */
  eglChooseConfig(display, attribs, nullptr, 0, &numConfigs);
  std::unique_ptr<EGLConfig[]> supportedConfigs(new EGLConfig[numConfigs]);
  assert(supportedConfigs);
  eglChooseConfig(display, attribs, supportedConfigs.get(), numConfigs,
                  &numConfigs);
  assert(numConfigs);
  auto i = 0;
  for (; i < numConfigs; i++) {
    auto& cfg = supportedConfigs[i];
    EGLint r, g, b, d;
    if (eglGetConfigAttrib(display, cfg, EGL_RED_SIZE, &r) &&
        eglGetConfigAttrib(display, cfg, EGL_GREEN_SIZE, &g) &&
        eglGetConfigAttrib(display, cfg, EGL_BLUE_SIZE, &b) &&
        eglGetConfigAttrib(display, cfg, EGL_DEPTH_SIZE, &d) && r == 8 &&
        g == 8 && b == 8 && d == 0) {
      config = supportedConfigs[i];
      break;
    }
  }
  if (i == numConfigs) {
    config = supportedConfigs[0];
  }

  if (config == nullptr) {
    LOGW("Unable to initialize EGLConfig");
    return -1;
  }

  /* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
   * guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
   * As soon as we picked a EGLConfig, we can safely reconfigure the
   * ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
  eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);
  surface =
      eglCreateWindowSurface(display, config, engine->app->window, nullptr);

  /* A version of OpenGL has not been specified here.  This will default to
   * OpenGL 1.0.  You will need to change this if you want to use the newer
   * features of OpenGL like shaders. */
  context = eglCreateContext(display, config, nullptr, nullptr);

  if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
    LOGW("Unable to eglMakeCurrent");
    return -1;
  }

  eglQuerySurface(display, surface, EGL_WIDTH, &w);
  eglQuerySurface(display, surface, EGL_HEIGHT, &h);

  engine->display = display;
  engine->context = context;
  engine->surface = surface;
  engine->width = w;
  engine->height = h;
  engine->state.angle = 0;

  // Check openGL on the system
  auto opengl_info = {GL_VENDOR, GL_RENDERER, GL_VERSION, GL_EXTENSIONS};
  for (auto name : opengl_info) {
    auto info = glGetString(name);
    LOGI("OpenGL Info: %s", info);
  }
  // Initialize GL state.
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
  glEnable(GL_CULL_FACE);
  glShadeModel(GL_SMOOTH);
  glDisable(GL_DEPTH_TEST);

  return 0;
}

/**
 * Tear down the EGL context currently associated with the display.
 */
static void engine_term_display(Engine* engine) {
  if (engine->display != EGL_NO_DISPLAY) {
    eglMakeCurrent(engine->display, EGL_NO_SURFACE, EGL_NO_SURFACE,
                   EGL_NO_CONTEXT);
    if (engine->context != EGL_NO_CONTEXT) {
      eglDestroyContext(engine->display, engine->context);
    }
    if (engine->surface != EGL_NO_SURFACE) {
      eglDestroySurface(engine->display, engine->surface);
    }
    eglTerminate(engine->display);
  }
  engine->Pause();
  engine->display = EGL_NO_DISPLAY;
  engine->context = EGL_NO_CONTEXT;
  engine->surface = EGL_NO_SURFACE;
}

/**
 * Process the next input event.
 */
static int32_t engine_handle_input(android_app* app,
                                   AInputEvent* event) {
  auto* engine = (Engine*)app->userData;
  if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
    engine->state.x = AMotionEvent_getX(event, 0);
    engine->state.y = AMotionEvent_getY(event, 0);
    return 1;
  }
  return 0;
}

/**
 * Process the next main command.
 */
static void engine_handle_cmd(android_app* app, int32_t cmd) {
  auto* engine = (Engine*)app->userData;
  switch (cmd) {
    case APP_CMD_SAVE_STATE:
      // The system has asked us to save our current state.  Do so.
      engine->app->savedState = malloc(sizeof(SavedState));
      *((SavedState*)engine->app->savedState) = engine->state;
      engine->app->savedStateSize = sizeof(SavedState);
      break;
    case APP_CMD_INIT_WINDOW:
      // The window is being shown, get it ready.
      if (engine->app->window != nullptr) {
        engine_init_display(engine);
      }
      break;
    case APP_CMD_TERM_WINDOW:
      // The window is being hidden or closed, clean it up.
      engine_term_display(engine);
      break;
    case APP_CMD_GAINED_FOCUS:
      // When our app gains focus, we start monitoring the accelerometer.
      if (engine->accelerometerSensor != nullptr) {
        ASensorEventQueue_enableSensor(engine->sensorEventQueue,
                                       engine->accelerometerSensor);
        // We'd like to get 60 events per second (in us).
        ASensorEventQueue_setEventRate(engine->sensorEventQueue,
                                       engine->accelerometerSensor,
                                       (1000L / 60) * 1000);
      }
      engine->Resume();
      break;
    case APP_CMD_LOST_FOCUS:
      // When our app loses focus, we stop monitoring the accelerometer.
      // This is to avoid consuming battery while not being used.
      if (engine->accelerometerSensor != nullptr) {
        ASensorEventQueue_disableSensor(engine->sensorEventQueue,
                                        engine->accelerometerSensor);
      }
      engine->Pause();
      break;
    default:
      break;
  }
}

int OnSensorEvent(int /* fd */, int /* events */, void* data) {
  CHECK_NOT_NULL(data);
  Engine* engine = reinterpret_cast<Engine*>(data);

  CHECK_NOT_NULL(engine->accelerometerSensor);
  ASensorEvent event;
  while (ASensorEventQueue_getEvents(engine->sensorEventQueue, &event, 1) > 0) {
    LOGI("accelerometer: x=%f y=%f z=%f", event.acceleration.x,
         event.acceleration.y, event.acceleration.z);
  }

  // From the docs:
  //
  // Implementations should return 1 to continue receiving callbacks, or 0 to
  // have this file descriptor and callback unregistered from the looper.
  return 1;
}

/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 */
void android_main(android_app* state) {
  Engine engine {};

  memset(&engine, 0, sizeof(engine));
  state->userData = &engine;
  state->onAppCmd = engine_handle_cmd;
  state->onInputEvent = engine_handle_input;
  engine.app = state;

  // Prepare to monitor accelerometer
  engine.CreateSensorListener(OnSensorEvent);

  if (state->savedState != nullptr) {
    // We are starting with a previous saved state; restore from it.
    engine.state = *(SavedState*)state->savedState;
  }

  while (!state->destroyRequested) {
    // Our input, sensor, and update/render logic is all driven by callbacks, so
    // we don't need to use the non-blocking poll.
    android_poll_source* source = nullptr;
    auto result = ALooper_pollOnce(-1, nullptr, nullptr,
                                   reinterpret_cast<void**>(&source));
    if (result == ALOOPER_POLL_ERROR) {
      fatal("ALooper_pollOnce returned an error");
    }

    if (source != nullptr) {
      source->process(state, source);
    }
  }

  engine_term_display(&engine);
}
// END_INCLUDE(all)
