/*
 * Copyright (C) The Android Open Source Project
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

#include <cstdio>
#include <cassert>
#include <android/native_window.h>
#include <android_native_app_glue.h>
#include <android/log.h>
#include "webp_decode.h"

#define  LOG_TAG    "libwebp-view"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGW(...)  __android_log_print(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

/*
 * webp files that are inside assets folder:
 *    they will be decoded and displayed as slide-show
 *    decoding will happen in its own thread
 */
const char * frames[] = {
        "clips/frame1.webp",
        "clips/frame2.webp",
        "clips/frame3.webp",
};
const int kFRAME_COUNT = sizeof(frames) / sizeof(frames[0]);
const int kFRAME_DISPLAY_TIME = 2;

/*
 * main object handles Android window frame update, and use webp to decode
 * pictures
 */
class Engine {
  public:
    explicit Engine(android_app* app) :
                app_(app),
                decoder_(nullptr),
                animating_(false) {
        memset(&frameStartTime_, 0, sizeof(frameStartTime_));
    }

    ~Engine() {}

    struct android_app* AndroidApp(void) const { return app_; }
    void StartAnimation(bool start) { animating_ = start; }
    bool IsAnimating(void) const { return animating_; }
    void TerminateDisplay(void) { StartAnimation(false); }

     // PrepareDrawing(): Initialize the Engine with current native window geometry
     //   and blank current screen to avoid garbbage displaying on device
    bool PrepareDrawing(void);

    // Update webp file into display window when it is decoded
    // and current frame has been displayed with requested time
    bool UpdateDisplay(void);

  private:
    void UpdateFrameBuffer(ANativeWindow_Buffer* buf, uint8_t* src);
    struct android_app* app_;
    WebpDecoder* decoder_;
    bool animating_;
    struct timespec frameStartTime_;
};

static int32_t ProcessAndroidInput(struct android_app *app, AInputEvent *event) {
    Engine* engine = reinterpret_cast<Engine*>(app->userData);
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        engine->StartAnimation(true);
        return 1;
    } else if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_KEY) {
        LOGI("Key event: action=%d keyCode=%d metaState=0x%x",
                AKeyEvent_getAction(event),
                AKeyEvent_getKeyCode(event),
                AKeyEvent_getMetaState(event));
    }

    return 0;
}

static void ProcessAndroidCmd(struct android_app *app, int32_t cmd) {
    static int32_t format = WINDOW_FORMAT_RGB_565;
    Engine* engine = reinterpret_cast<Engine*>(app->userData);
    switch (cmd) {
        case APP_CMD_INIT_WINDOW:
            if (engine->AndroidApp()->window != NULL) {
                // save current format to format variable, and set
                // display format to 565 to save time coping. normally
                // 565 might be buggy ( if 565 works, 32 bit mostly like
                // would work )
                format = ANativeWindow_getFormat(app->window);
                ANativeWindow_setBuffersGeometry(app->window,
                              ANativeWindow_getWidth(app->window),
                              ANativeWindow_getHeight(app->window),
                              WINDOW_FORMAT_RGB_565);
                engine->PrepareDrawing();
                engine->UpdateDisplay();
                engine->StartAnimation(true);
            }
            break;
        case APP_CMD_TERM_WINDOW:
            engine->StartAnimation(false);
            engine->TerminateDisplay();
            ANativeWindow_setBuffersGeometry(app->window,
                          ANativeWindow_getWidth(app->window),
                          ANativeWindow_getHeight(app->window),
                          format);
            break;
        case APP_CMD_LOST_FOCUS:
            engine->StartAnimation(false);
            engine->UpdateDisplay();
            break;
    }
}

// Android application glue entry function for us
extern "C" void android_main(struct android_app* state) {

    Engine engine(state);

    state->userData = reinterpret_cast<void*>(&engine);
    state->onAppCmd = ProcessAndroidCmd;
    state->onInputEvent = ProcessAndroidInput;

    // loop waiting for stuff to do.
    while (1) {
        // Read all pending events.
        int ident;
        int events;
        struct android_poll_source* source;

        // If not animating, we will block forever waiting for events.
        // If animating, we loop until all events are read, then continue
        // to draw the next frame of animation.
        while ((ident = ALooper_pollAll(engine.IsAnimating() ? 0 : -1, NULL, &events,
                (void**)&source)) >= 0) {

            // Process this event.
            if (source != NULL) {
                source->process(state, source);
            }

            // Check if we are exiting.
            if (state->destroyRequested != 0) {
                LOGI("Engine thread destroy requested!");
                engine.TerminateDisplay();
                return;
            }
        }

        if (engine.IsAnimating()) {
            engine.UpdateDisplay();
        }
    }
}

// Engine class implementations
bool Engine::PrepareDrawing(void) {
    // create decoder
    if (decoder_) {
        decoder_->DestroyDecoder();
    }
    ANativeWindow_Buffer buf;
    if (ANativeWindow_lock(app_->window, &buf, NULL) < 0) {
        LOGW("Unable to lock window buffer to create decoder");
        return false;
    }
    UpdateFrameBuffer(&buf, nullptr);
    ANativeWindow_unlockAndPost(app_->window);
    DecodeSurfaceDescriptor descriptor;
    switch (buf.format) {
        case  WINDOW_FORMAT_RGB_565:
            descriptor.format_ = SurfaceFormat::SURFACE_FORMAT_RGB_565;
            break;
        case WINDOW_FORMAT_RGBX_8888:
            descriptor.format_ = SurfaceFormat::SURFACE_FORMAT_RGBX_8888;
            break;
        case WINDOW_FORMAT_RGBA_8888:
            descriptor.format_ = SurfaceFormat::SURFACE_FORMAT_RGBA_8888;
            break;
        default:
            return false;
    }
    descriptor.width_  = buf.width;
    descriptor.height_ = buf.height;
    descriptor.stride_ = buf.stride;

    decoder_ = new WebpDecoder(frames, kFRAME_COUNT, &descriptor,
                               app_->activity->assetManager);
    assert(decoder_);
    if (!decoder_) {
        return false;
    }
    decoder_->DecodeFrame();

    return true;
}

/*
 * Only copy decoded webp picture when:
 *  - current frame has been on for kFrame_DISPLAY_TIME seconds
 *  - a new picture is decoded
 * After copying, start decoding the next frame
 */
bool Engine::UpdateDisplay(void) {
    if (!app_->window || !decoder_) {
        assert(0);
        return false;
    }
    struct timespec curTime;
    clock_gettime(CLOCK_MONOTONIC, &curTime);
    if (curTime.tv_sec <
        (frameStartTime_.tv_sec + (__kernel_time_t )kFRAME_DISPLAY_TIME)) {
        // current frame is displayed less than required duration
        return false;
    }
    uint8_t *frame = decoder_->GetDecodedFrame();
    if (!frame)
        return false;

    ANativeWindow_Buffer buffer;
    if (ANativeWindow_lock(app_->window, &buffer, nullptr) < 0) {
        LOGW("Unable to lock window buffer");
        return false;
    }
    UpdateFrameBuffer(&buffer, frame);
    ANativeWindow_unlockAndPost(app_->window);
    clock_gettime(CLOCK_MONOTONIC, &frameStartTime_);

    // start decoding next frame
    decoder_->DecodeFrame();
    return true;
}

/*
 * UpdateFrameBuffer():
 *     Internal function to perform bits copying onto current frame buffer
 *     src:
 *        - if nullptr, blank it
 *        - otherwise,  copy to given buf
 *     assumption:
 *         src and bug MUST be in the same geometry format & layout
 */
void Engine::UpdateFrameBuffer(ANativeWindow_Buffer* buf, uint8_t* src) {
    // src is either null: to blank the screen
    //     or holding exact pixels with the same fmt [stride is the SAME]
    uint8_t *dst = reinterpret_cast<uint8_t*> (buf->bits);
    uint32_t bpp;
    switch (buf->format) {
        case WINDOW_FORMAT_RGB_565:
            bpp = 2;
            break;
        case WINDOW_FORMAT_RGBA_8888:
        case WINDOW_FORMAT_RGBX_8888:
            bpp = 4;
            break;
        default:
            assert( 0 );
            return;
    }
    uint32_t stride, width;
    stride = buf->stride * bpp;
    width = buf->width * bpp;
    if (src) {
        for (auto height =0; height < buf->height; ++height) {
            memcpy(dst, src, width);
            dst += stride, src += stride;
        }
    } else {
        for (auto height =0; height < buf->height; ++height) {
            memset(dst, 0, width);
            dst += stride;
        }
    }
}
