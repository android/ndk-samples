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
#pragma once
#include <queue>
#include <mutex>
#include <atomic>
#include <android/asset_manager.h>
#include <android/imagedecoder.h>

enum class DecodeState { IDLE, BUSY, COMPLETED };
enum class SurfaceFormat : unsigned int {
    SURFACE_FORMAT_RGBA_8888,
    SURFACE_FORMAT_RGBX_8888,
    SURFACE_FORMAT_RGB_565,
    SURFACE_FORMAT_YUV_420 // Not implemented yet
};
struct DecodeSurfaceDescriptor {
    // surface size in pixels
    int32_t width, height, stride;
    SurfaceFormat format;
};

struct DecodeFrameDescriptor {
    int32_t width;
    int32_t height;
    int32_t stride;
    uint8_t* _Nullable bits;
    int32_t rotation;
    uint64_t displayDuration;
};

/*
 * ImageDecoder:
 *     Select a picture to decode in its own thread. The thread is deleted
 *     once a picture is decoded into its internal memory. The next picture
 *     will be decoded when:
 *       - the decoded frame is retrieved
 *       - Another DecodeFrame() is called [this is trigger]
 *    when display format changes, call DestroyDecoder() to release this decoder
 *    and allocate a new decoder object.
 */
class ImageDecoder {
  public:
    explicit ImageDecoder(const char* _Nonnull * _Nonnull files, uint32_t count,
                          DecodeSurfaceDescriptor * _Nullable frameBuf,
                          AAssetManager* _Nonnull assetMgr);

    /**
     * Report whether the decoder is currently decoding frames.
     * Caller should:
     *    - call isBusy() to make sure decoder is not busy
     *    - call DecoderNextImage() to kick start decoding
     *    - [Optional] call isBusy() to make sure frame is decoded
     *    - call getDecodedFrame() to retrieve the decoded bits
     *    - call DecodeNextImage() again to kick start the next frame decoding
     *    ...
     */
    bool IsBusy(void);

    /**
     * Kick start to decode the next image(or frame). Refer to isBusy() for usage.
     * This is non-re-entrant function: do not call if previous frame is not completed
     * yet.
     * return bool: true - decoding started,
     *              false - error happened, decoding could not proceed
     */
    bool DecodeNextImage(void);

    /**
     * Report the decoded frame info.
     * param frame - pointer to data structure for decoded frame info
     * return bool:
     *   true - image successfully copied into the given frame.
     *   false - image is not ready (not decoded yet).
     */
    bool GetDecodedFrame(DecodeFrameDescriptor* _Nonnull frame);

    ~ImageDecoder();

private:
    DecodeSurfaceDescriptor bufInfo;
    AAssetManager * _Nonnull assetMgr;
    std::queue<const char* _Nonnull> fileNames;
    const char*  _Nullable curImgFile;
    AAsset* _Nullable imgAsset;
    AImageDecoder*  _Nullable decoder;
    const AImageDecoderHeaderInfo* _Nullable headerInfo;
    AImageDecoderFrameInfo* _Nullable frameInfo;
    uint64_t frameDuration;

    std::atomic<DecodeState> decodeState;
    std::vector<uint8_t>    decodedImageBits;
    int32_t  decodedImageStride;
    ARect    targetRect;
    int32_t  rotationAngle;

    uint32_t   bytePerPixel;

    std::mutex decodeMutex;
    AndroidBitmapFormat GetDecodingBitmapFormat();
};
