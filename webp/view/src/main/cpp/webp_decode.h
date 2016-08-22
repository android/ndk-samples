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
#ifndef __WEBP_DECODE_H__
#define __WEBP_DECODE_H__
#include <queue>
#include <android/asset_manager.h>

enum DecodeState { state_idle, state_decoding, state_ready};
enum class SurfaceFormat : unsigned int {
    SURFACE_FORMAT_RGBA_8888,
    SURFACE_FORMAT_RGBX_8888,
    SURFACE_FORMAT_RGB_565,
    SURFACE_FORMAT_YUV_420 // Not implemented yet
};
struct DecodeSurfaceDescriptor {
    // surface size in pixels
    int32_t width_, height_, stride_;
    SurfaceFormat format_;
};

/*
 * Webp decoder wrapper:
 *     Select a picture to decode in its own thread. The thread is deleted
 *     once a picture is decoded into its internal memory. The next picture
 *     will be decoded when:
 *       - the decoded frame is retrieved
 *       - Another DecodeFrame() is called [this is trigger]
 *    when display format changes, call DestroyDecoder() to release this decoder
 *    and allocate a new deocder object.
 */
class WebpDecoder {
  public:
    explicit WebpDecoder(const char** files, uint32_t count,
                         DecodeSurfaceDescriptor* surfDesc,
                         AAssetManager* assetMgr);
    // Start decode a picture
    bool     DecodeFrame(void);

    // Poll to see if a picture is decoded and ready to be used/displayed
    uint8_t *GetDecodedFrame(void);

    // WebpDecoder internal decoding function, no called from user
    void     DecodeFrameInternal(void);

    // Release this decoder after usage
    bool     DestroyDecoder(void);

  private:
    DecodeSurfaceDescriptor bufInfo_;
    AAssetManager*          assetMgr_;
    std::queue<const char*> files_;
    uint8_t*  buf_;
    bool      stopPending_;
    DecodeState state_;

    uint32_t   bytePerPix_;
    pthread_t* worker_;
    /*
     * private destructor prevent object directly call delete
     */
    ~WebpDecoder();
};
#endif // __WEBP_DECODE_H__