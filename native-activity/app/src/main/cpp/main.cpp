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

#include <android/choreographer.h>
#include <android/hardware_buffer.h>
#include <android/log.h>
#include <android/native_window.h>
#include <android/set_abort_message.h>
#include <android_native_app_glue.h>

#include <chrono>
#include <cstdlib>
#include <memory>

using namespace std::literals::chrono_literals;

#define LOG_TAG "native-activity"

#define _LOG(priority, fmt, ...)                    \
  ((void)__android_log_print((priority), (LOG_TAG), \
                             (fmt)__VA_OPT__(, ) __VA_ARGS__))

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

// Note: little endian, the opposite of normal hex color codes. ABGR, rather
// than RGBA.
enum class Color : uint32_t {
  kRed = 0x000000ff,
  kGreen = 0x0000ff00,
  kBlue = 0x00ff0000,
};

/**
 * Shared state for our app.
 */
class Engine {
 public:
  explicit Engine(android_app* app) : app_(app) {}

  void AttachWindow() {
    if (ANativeWindow_setBuffersGeometry(
            app_->window, 0, 0, AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM) < 0) {
      LOGE("Unable to set window buffer geometry");
      window_initialized = false;
      return;
    }
    window_initialized = true;
    color_ = Color::kRed;
    last_update_ = std::chrono::steady_clock::now();
  }

  void DetachWindow() { window_initialized = false; }

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
  android_app* app_;
  bool window_initialized = false;
  bool running_ = false;
  Color color_ = Color::kRed;
  std::chrono::time_point<std::chrono::steady_clock> last_update_;

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

    // Choreographer does not continuously schedule the callback. We have to re-
    // register the callback each time we're ticked.
    ScheduleNextTick();
    Update();
    DrawFrame();
  }

  void Update() {
    auto now = std::chrono::steady_clock::now();
    if (now - last_update_ > 1s) {
      switch (color_) {
        case Color::kRed:
          color_ = Color::kGreen;
          break;
        case Color::kGreen:
          color_ = Color::kBlue;
          break;
        case Color::kBlue:
          color_ = Color::kRed;
          break;
        default:
          fatal("unexpected color %08x", static_cast<uint32_t>(color_));
      }
      last_update_ = now;
    }
  }

  void DrawFrame() const {
    if (app_->window == nullptr) {
      LOGE("Attempted to draw frame but there is no window attached");
      return;
    }

    ANativeWindow_Buffer buffer;
    if (ANativeWindow_lock(app_->window, &buffer, nullptr) < 0) {
      LOGE("Unable to lock window buffer");
      return;
    }

    if (!window_initialized) {
      // If for some reason we were not able to initialize the window geometry,
      // then we can't assume the buffer format. We could detect the buffer's
      // format and adjust our buffer fill here to accommodate that, but's a bit
      // beyond the scope of this sample.
      return;
    }

    for (auto y = 0; y < buffer.height; y++) {
      for (auto x = 0; x < buffer.width; x++) {
        size_t pixel_idx = y * buffer.stride + x;
        reinterpret_cast<uint32_t*>(buffer.bits)[pixel_idx] =
            static_cast<uint32_t>(color_);
      }
    }

    ANativeWindow_unlockAndPost(app_->window);
  }
};

/**
 * Process the next main command.
 */
static void engine_handle_cmd(android_app* app, int32_t cmd) {
  auto* engine = (Engine*)app->userData;
  switch (cmd) {
    case APP_CMD_INIT_WINDOW:
      engine->AttachWindow();
      break;
    case APP_CMD_TERM_WINDOW:
      engine->DetachWindow();
      break;
    case APP_CMD_GAINED_FOCUS:
      engine->Resume();
      break;
    case APP_CMD_LOST_FOCUS:
      engine->Pause();
      break;
    default:
      break;
  }
}

/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 */
void android_main(android_app* state) {
  Engine engine{state};

  state->userData = &engine;
  state->onAppCmd = engine_handle_cmd;

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
}
