/*
 * Copyright (C) 2021 The Android Open Source Project
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
#include "image_viewer.h"
#include "image_decoder.h"
#include "display_timer.h"
#include "android_debug.h"

/*
 * Compressed files that are inside assets folder:
 *    they will be decoded and displayed as slide-show
 *    decoding will happen in its own thread
 */
const char * imageFiles[] = {
        "images/android.webp",
        "images/android.avif",
        "images/android.heic",
};
const int kFRAME_COUNT = sizeof(imageFiles) / sizeof(imageFiles[0]);

ImageViewer::ImageViewer(android_app* _Nullable app) :
    androidApp(app),
    imgDecoder(nullptr),
    animatingInProgress(false) {

}

ImageViewer::~ImageViewer() {
    delete imgDecoder;
}

struct android_app* _Nullable ImageViewer::GetAndroidApp(void) const{
    return androidApp;
}
void ImageViewer::StartAnimation(bool start) {
    animatingInProgress = start;
}
bool ImageViewer::IsAnimating(void) const {
    return animatingInProgress;
}
void ImageViewer::TerminateDisplay(void) {
    StartAnimation(false);
}

// Engine class implementations
bool ImageViewer::PrepareDrawing(void) {
    // create decoder
    if (imgDecoder) {
        delete imgDecoder;
        imgDecoder = nullptr;
    }
    ANativeWindow_Buffer buf;
    if (ANativeWindow_lock(androidApp->window, &buf, NULL) < 0) {
        LOGW("Unable to lock window buffer to create decoder");
        return false;
    }
    ClearFrameBuffer(&buf);
    ANativeWindow_unlockAndPost(androidApp->window);

    DecodeSurfaceDescriptor descriptor;
    switch (buf.format) {
        case  WINDOW_FORMAT_RGB_565:
            descriptor.format = SurfaceFormat::SURFACE_FORMAT_RGB_565;
            break;
        case WINDOW_FORMAT_RGBX_8888:
            descriptor.format = SurfaceFormat::SURFACE_FORMAT_RGBX_8888;
            break;
        case WINDOW_FORMAT_RGBA_8888:
            descriptor.format = SurfaceFormat::SURFACE_FORMAT_RGBA_8888;
            break;
        default:
            return false;
    }
    descriptor.width  = buf.width;
    descriptor.height = buf.height;
    descriptor.stride = buf.stride;

    imgDecoder = new ImageDecoder(imageFiles, kFRAME_COUNT, &descriptor, androidApp->activity->assetManager);

    if (!imgDecoder) {
        LOGE("Unable to create ImageDecoder instance in %s", __FUNCTION__);
        return false;
    }

    // Kick start the 1st image decoding
    imgDecoder->DecodeNextImage();

    return true;
}

/**
 * Only copy decoded webp picture when:
 *  - current frame has been on for kFrame_DISPLAY_TIME seconds
 *  - a new picture is decoded
 * After copying, start decoding the next frame
 */
bool ImageViewer::UpdateDisplay(void) {
    if (!androidApp->window || !imgDecoder) {
        LOGE("Decoder(%p) or NativeWindow(%p) is not ready in %s",
              androidApp->window, imgDecoder, __FUNCTION__);
        return false;
    }

    if(!displayTimer.IsExpired() || imgDecoder->IsBusy()) {
        // current frame is displayed less than required duration or image is not ready
        return false;
    }

    DecodeFrameDescriptor rawImg;
    if(!imgDecoder->GetDecodedFrame(&rawImg))
        return false;

    ANativeWindow_Buffer buffer;
    if (ANativeWindow_lock(androidApp->window, &buffer, nullptr) < 0) {
        LOGW("Unable to lock window buffer");
        return false;
    }
    ClearFrameBuffer(&buffer); // clear the screen
    UpdateFrameBuffer(&buffer, &rawImg);
    ANativeWindow_unlockAndPost(androidApp->window);

    displayTimer.Reset(rawImg.displayDuration);

    imgDecoder->DecodeNextImage();
    return true;
}

/**
 * Internal function to translate ANativeWindow format to byte per pixel.
 * @param buf: points to the ANativeWindow format.
 * @return int32_t: byte per pixel for the give ANativeWindow
 */
int32_t ImageViewer::getNativeWindowBpp(ANativeWindow_Buffer* _Nonnull buf) {
    uint32_t bpp = 0;
    switch (buf->format) {
        case WINDOW_FORMAT_RGB_565:
            bpp = 2;
            break;
        case WINDOW_FORMAT_RGBA_8888:
        case WINDOW_FORMAT_RGBX_8888:
            bpp = 4;
            break;
        default:
            LOGE("Could not recognize the NativeWindow format: %d in %s",
                 buf->format, __FUNCTION__);
            assert(0);;
    }

    return bpp;
}

/**
 * ClearFrameBuffer: zero out the framebuffer
 * @param buf: the display buffer to clear
 */
void ImageViewer::ClearFrameBuffer(ANativeWindow_Buffer * _Nonnull buf) {
    uint8_t *dst = reinterpret_cast<uint8_t *> (buf->bits);

    int32_t bpp = getNativeWindowBpp(buf);
    uint32_t dstStride, width;
    dstStride = buf->stride * bpp;
    width = buf->width * bpp;

    // blank the screen
    for (auto height = 0; height < buf->height; ++height) {
        memset(dst, 0, width);
        dst += dstStride;
    }
}
/**
 * UpdateFrameBuffer():
 *     Internal function to perform bits copying onto current frame buffer
 * @param buf: already locked-down target NativeWindow to update,
 * @param frame: already decoded raw image to display.
 *        - if nullptr, clear screen
 *        - otherwise, pixel by pixel copy with letterbox enabled
 */
void ImageViewer::UpdateFrameBuffer(ANativeWindow_Buffer * _Nonnull buf,
        DecodeFrameDescriptor* _Nonnull frame) {

    uint8_t *dst = reinterpret_cast<uint8_t *> (buf->bits);

    int32_t bpp = getNativeWindowBpp(buf);
    uint32_t dstStride, width;
    dstStride = buf->stride * bpp;
    width = buf->width * bpp;

    // perform pixel by pixel copying.
    uint8_t *src = frame->bits;
    int32_t start_x;
    int32_t start_y;
    if(frame->rotation == 0 || frame->rotation == 180) {
        start_x = (buf->width - frame->width) / 2;
        start_y = (buf->height - frame->height) / 2;
    } else if (frame->rotation == 90 || frame->rotation == 270){
        // need to rotate 90 or 270 degree
        start_x = (buf->width - frame->height) / 2;
        start_y = (buf->height - frame->width) / 2;
    } else {
        LOGE("Wrong frame->rotation(%d) to %s", frame->rotation, __FUNCTION__ );
        return;
    }
    assert(start_x >= 0 && start_y >= 0);
    dst += (start_y * dstStride) + (start_x * bpp);

    // line by line copying for 0 degree rotation
    if(frame->rotation == 0) {
        for (auto height = 0; height < frame->height; ++height) {
            memcpy(dst, src, width);
            dst += dstStride, src += frame->stride;
        }
        return;
    }

    if(frame->rotation == 180) {
        src += frame->stride * (frame->height - 1);
        for (auto height = 0; height < frame->height; ++height) {
            memcpy(dst, src, width);
            dst += dstStride, src -= frame->stride;
        }
        return;
    }

    // one pixel at a time, rotate on the fly
    if (frame->rotation == 90) {
        // one pixel at a time, rotate on the fly
        src += (frame->width - 1) * bpp;

        for (auto height = 0; height < frame->width; height++) {
            uint8_t *srcLine = src;
            uint8_t *dstLine = dst;
            for (auto w = 0; w < frame->height; w++) {
                memcpy(dstLine, srcLine, bpp);

                dstLine += bpp;             // advance one pixel
                srcLine += frame->stride;   // move down a line
            }
            dst += dstStride;  // advance 1 line
            src -= bpp;        // move 1 pixel backward for the next source column
        }
        return;
    }

    if(frame->rotation == 270) {
        src += (frame->height - 1) * frame->stride;
        for (auto height = 0; height < frame->width; height++) {
            uint8_t *srcLine = src;
            uint8_t *dstLine = dst;
            for (auto w = 0; w < frame->height; w++) {
                memcpy(dstLine, srcLine, bpp);
                dstLine += bpp;             // advance one pixel
                srcLine -= frame->stride;   // move up a line
            }

            dst += dstStride;  // advance 1 line
            src += bpp;        // move 1 pixel forward for the next source column
        }
        return;
    }
}

