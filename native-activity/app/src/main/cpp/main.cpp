// Copyright (C) 2010 The Android Open Source Project
// SPDX-License-Identifier: Apache-2.0

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
 * The implementation for our app.
 *
 * This class implements the activity lifecycle behaviors akin to how Activity
 * would in a Java app. With native_app_glue, those lifecycle events are instead
 * communicated to this class from engine_handle_cmd, which is in turned called
 * by looper (see the description below in android_main).
 *
 * The comments here will briefly explain some aspects of the Android activity
 * lifecycle, but they cannot explain it fully. See
 * https://developer.android.com/guide/components/activities/activity-lifecycle
 * and the other docs in that section for more information.
 */
class Engine {
 public:
  explicit Engine(android_app* app) : app_(app) {}

  void AttachWindow() {
    // This is called whenever a new native window is created for our app, so we
    // need to reinitialize the buffer format to the format our render loop
    // expects.
    //
    // Attaching the window will not cause the app to start running its update
    // and render loop. The app's update cycle is separately enabled by
    // Engine::Resume.
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

  void DetachWindow() {
    // This is called whenever the native window for our app is destroyed. That
    // does not necessarily mean that the app is being killed, as it is also
    // done when the screen rotates.
    //
    // For a more typical app where the rendering is done with OpenGL or Vulkan,
    // this is where you'd perform any window cleanup needed by those
    // frameworks. For our app, it's sufficient to just set a flag to disable
    // our render loop.
    window_initialized = false;
  }

  /// Resumes ticking the application.
  void Resume() {
    // This is called whenever the activity is resumed (brought into the
    // foreground). When that happens, we schedule our next update tick with
    // Choreographer. Choreographer is the Android system that paces app render
    // loops. If you instead render new frames in a loop without frame pacing,
    // you risk rendering more quickly than the display pipeline is able to
    // present new frames. This will increase the latency between frame
    // submission and presentation.
    // https://developer.android.com/ndk/reference/group/choreographer

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
  void Pause() {
    // This is called whenever something interrupts the activity and moves it
    // into the background. In multiwindow mode the app might still be visible,
    // but it is no longer the focused app and should pause accordingly,
    running_ = false;
  }

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

    // Lock the native window's buffer so we can write to it.
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

    // Write a solid color to the window buffer.
    for (auto y = 0; y < buffer.height; y++) {
      for (auto x = 0; x < buffer.width; x++) {
        // Note that we index the row by the buffers stride, not its width. The
        // buffer itself may be wider than the render area.
        size_t pixel_idx = y * buffer.stride + x;
        reinterpret_cast<uint32_t*>(buffer.bits)[pixel_idx] =
            static_cast<uint32_t>(color_);
      }
    }

    // Now unlock the buffer, causing the display to update.
    ANativeWindow_unlockAndPost(app_->window);
  }
};

/**
 * The callback for native_app_glue's Activity lifecycle event queue.
 */
static void engine_handle_cmd(android_app* app, int32_t cmd) {
  auto* engine = (Engine*)app->userData;
  // There are a lot of lifecycle events that we're ignoring here. See
  // android_native_app_glue.h for the complete list that native_app_glue
  // handles (which may not be complete if Activity adds new lifecycle
  // methods!)
  //
  // Most applications will need to handle many more of than just this set.
  // We're getting away with ignoring most events because this app doesn't do
  // anything interesting.
  switch (cmd) {
    case APP_CMD_INIT_WINDOW:
      // https://developer.android.com/ndk/reference/struct/a-native-activity-callbacks#onnativewindowcreated
      engine->AttachWindow();
      break;
    case APP_CMD_TERM_WINDOW:
      // https://developer.android.com/ndk/reference/struct/a-native-activity-callbacks#onnativewindowdestroyed
      engine->DetachWindow();
      break;
    case APP_CMD_GAINED_FOCUS:
      // https://developer.android.com/ndk/reference/struct/a-native-activity-callbacks#onwindowfocuschanged
      engine->Resume();
      break;
    case APP_CMD_LOST_FOCUS:
      // https://developer.android.com/ndk/reference/struct/a-native-activity-callbacks#onwindowfocuschanged
      engine->Pause();
      break;
    default:
      break;
  }
}

/**
 * `android_main()` is the entry point for an app using `native_app_glue`.
 *
 * This function is called from a separate thread spawned from
 * `ANativeActivity_onCreate`, which is the native equivalent of the
 * `onCreate` stage in the activity lifecycle:
 * https://developer.android.com/guide/components/activities/activity-lifecycle
 *
 * The `android_main()` implementation typically will perform application setup,
 * enter the main event loop, and shut down if necessary.
 */
void android_main(android_app* state) {
  Engine engine{state};

  // onAppCmd is called whenever native_app_glue receives one of the activity
  // lifecycle events from the framework:
  // https://developer.android.com/guide/components/activities/activity-lifecycle
  //
  // Typical native Android applications would implement the various
  // onPause(), onResume(), etc in JNI methods. native_app_glue handles that
  // for us and instead presents those method calls as if they were a pollable
  // event queue. Our engine_handle_cmd callback is the function that will
  // respond to new events in that queue.
  state->onAppCmd = engine_handle_cmd;

  // The userData property will be passed to the callback we registered with
  // onAppCmd.
  state->userData = &engine;

  // destroyRequested will be set when onDestroy() is called:
  // https://developer.android.com/guide/components/activities/activity-lifecycle#ondestroy
  while (!state->destroyRequested) {
    // native_app_glue communicates events to the app using Looper rather than
    // method calls like a Java activity would use. Looper is an Android API
    // similar to POSIX's select(2):
    // https://developer.android.com/ndk/reference/group/looper#alooper
    //
    // Whenever an activity lifecycle method is called on our ANativeActivity,
    // or an input event is received, native_app_glue will forward that to our
    // app as a looper event.
    //
    // Polling looper can be done in either a blocking or non-blocking manner.
    // If your app needs to wake periodically on this thread, pass a value for
    // the poll timeout. Most of the things you'd normally do during this loop
    // (respond to input or sensor updates, or even render the next frame of
    // your game) should be driven by callbacks registered with those
    // subsystems though rather than done here. Our main loop doesn't need to
    // do anything other than process looper events.
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

  // Most cleanup code should actually run in response to activity lifecycle
  // events that are processed in engine_handle_cmd rather than after the main
  // loop exits, as would be more typical of main loops on desktop platforms.
}
