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
#include <cassert>
#include <pthread.h>
#include <webp/decode.h>
#include "webp_decode.h"

WebpDecoder::WebpDecoder(const char** files, uint32_t count,
                         DecodeSurfaceDescriptor* frameBuf,
                         AAssetManager* assetMgr)
    : assetMgr_(assetMgr),
      buf_(nullptr), stopPending_(false),
      state_(state_idle), worker_(nullptr) {
    for (auto i = 0; i < count; i++) {
        files_.push(files[i]);
    }
    if (count && assetMgr && frameBuf) {
        bufInfo_ = *frameBuf;
        switch (bufInfo_.format_) {
            case SurfaceFormat::SURFACE_FORMAT_RGB_565:
                bytePerPix_ = 2;
                break;
            case SurfaceFormat::SURFACE_FORMAT_RGBA_8888:
            case SurfaceFormat::SURFACE_FORMAT_RGBX_8888:
                bytePerPix_ = 4;
                break;
            default:
                assert(0);
                return;
        }
        // allocate a private decode buffer
        uint32_t size = bufInfo_.height_ * bufInfo_.stride_ * bytePerPix_;
        buf_ = new uint8_t [size];
        assert(buf_);
    }
}

/*
 * GetDecodedFrame():  return decoded frame if available,
 *                     return nullptr otherwise
 */
uint8_t* WebpDecoder::GetDecodedFrame(void) {
    return (state_ == state_ready ? buf_ : nullptr);
}

/*
 * DecodeFrame():
 *    thread function to decode a picture
 *    directly pass through to internal decoding function
 */
void* DecodeFrame(void * decoder) {
    reinterpret_cast<WebpDecoder*>(decoder)->DecodeFrameInternal();
    return nullptr;
}

/*
 * DecodeFrameInternal():
 *    Main decoding function from app side, and executing inside its own thread
 */
void WebpDecoder::DecodeFrameInternal() {
    const char * webpFile = files_.front();
    files_.pop();
    files_.push(webpFile);

    AAsset* frameFile = AAssetManager_open(assetMgr_, webpFile, AASSET_MODE_BUFFER);
    assert(frameFile != NULL);
    int32_t len = AAsset_getLength(frameFile);
    uint8_t *buf = new uint8_t[len];
    assert(buf);
    len = AAsset_read(frameFile, buf, len);
    assert(len > 0);
    AAsset_close(frameFile);

    WebPDecoderConfig config;
    if (!WebPInitDecoderConfig(&config)) {
        assert(0);
    }

    VP8StatusCode  status = WebPGetFeatures(buf, len, &config.input);
    assert(status == VP8_STATUS_OK);

    // let's decode it into a buffer ...
    config.options.bypass_filtering = 1;
    config.options.no_fancy_upsampling = 1;
    config.options.flip = 0;
    config.options.use_scaling = 1;
    config.options.scaled_width = bufInfo_.width_;
    config.options.scaled_height = bufInfo_.height_;

    // this does not seems to have difference on Nexus 5
    config.options.use_threads = 1;
    switch (bufInfo_.format_) {
        case SurfaceFormat::SURFACE_FORMAT_RGB_565:
            config.output.colorspace = MODE_RGB_565;
            break;
        case SurfaceFormat::SURFACE_FORMAT_RGBA_8888:
        case SurfaceFormat::SURFACE_FORMAT_RGBX_8888:
            config.output.colorspace = MODE_RGBA;
            break;
        default:
            assert( 0 );
            delete  [] buf;
            return;
    }
    config.output.width = bufInfo_.width_;
    config.output.height = bufInfo_.height_;
    config.output.is_external_memory = 1;
    config.output.private_memory = reinterpret_cast<uint8_t*>(buf_);
    config.output.u.RGBA.stride = bufInfo_.stride_ * bytePerPix_;
    config.output.u.RGBA.rgba  = config.output.private_memory;
    config.output.u.RGBA.size  = config.output.height *
                                 config.output.u.RGBA.stride;

    status = WebPDecode(buf, len, &config);
    WebPFreeDecBuffer(&config.output);
    delete [] buf;

    // only change state when it is decoded OK. Our small decoder engine will be
    // in decoding state when error happens
    assert(status == VP8_STATUS_OK);
    if (status == VP8_STATUS_OK) {
        state_ = state_ready;
    }

    delete worker_;
    worker_ = nullptr;

    // if we were asked to release while we are busy decoding (at this point,
    // this is dangling pointer), we perform the release here to complete the
    // request.
    if (stopPending_) {
        stopPending_ = false;
        delete  this;
    }
}

/*
 * DecodeFrame(void):
 *     Start to decode a picture into internal frame memory
 *     The internal memory layout and size are the same as andriod native
 *     window to save copying when possible.
 *
 *     The decoded frames are scaled up/down by webp decoder to fix the display
 *     window size.
 */
bool WebpDecoder::DecodeFrame(void) {
    if (state_ == state_decoding)
        return false;
    pthread_t       worker_;
    pthread_attr_t  attrib;
    pthread_attr_init( &attrib);
    pthread_attr_setdetachstate(&attrib, PTHREAD_CREATE_DETACHED);
    int status = pthread_create(&worker_, &attrib, ::DecodeFrame, this);
    pthread_attr_destroy(&attrib);

    if (status == 0) {
        state_ = state_decoding;
        return true;
    }

    // create thread failed...
    assert(false);
    return false;
}

/*
 * DestroyDecoder(void):
 *     Self-delete if decoder is idle
 *     Otherwise set up a flag to let decoding thread perform self-delete when
 *     it finishes. Upon returning from the function, the class pointer is invalid
 *     and should not be used
 */
bool WebpDecoder::DestroyDecoder(void) {
    if (state_ == state_decoding) {
        stopPending_ = true;
        return false;
    }

    delete this;
    return true;
}

/*
 * private destructor prevent object directly call delete
 */
WebpDecoder::~WebpDecoder() {
    if (buf_)  delete [] buf_;
    buf_ = nullptr;
}

