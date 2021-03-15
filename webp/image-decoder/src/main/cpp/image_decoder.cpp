/*
 * Copyright (C) 2021  The Android Open Source Project
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

#include <android/imagedecoder.h>
#include <android/data_space.h>
#include "image_decoder.h"
#include "image_scale_descriptor.h"
#include "android_debug.h"
#include <cassert>
#include <thread>

// Default displaying time image for each frame, used when switching from one image file to
// the next image file at run time:
//  - frames inside the animated files would provide the display time, so kDEFAULT_DURATION
//    is not needed
//  - non-animated image files will need this.
const uint64_t kDEFAULT_DURATION = (2 * 1000000000UL);  // 2 second

/*
 * ImageDecoder constructor:
 */
ImageDecoder::ImageDecoder(const char** files, uint32_t count,
                           DecodeSurfaceDescriptor *frameBuf,
                           AAssetManager* mgr)
    : assetMgr(mgr), curImgFile(nullptr), imgAsset(nullptr),
      decoder(nullptr),
      headerInfo(nullptr), frameInfo(nullptr), frameDuration(0UL),
      decodeState(DecodeState::IDLE),
      decodedImageStride(0), rotationAngle(0) {

    for (auto i = 0; i < count; i++) {
      fileNames.push(files[i]);
    }

    bufInfo = *frameBuf;
    if (count && frameBuf) {
        switch (bufInfo.format) {
            case SurfaceFormat::SURFACE_FORMAT_RGB_565:
                bytePerPixel = 2;
                break;
            case SurfaceFormat::SURFACE_FORMAT_RGBA_8888:
            case SurfaceFormat::SURFACE_FORMAT_RGBX_8888:
                bytePerPixel = 4;
                break;
            default:
                ASSERT(false, "Unsupported display buffer format(%d) to %s",
                       bufInfo.format, __FUNCTION__);
                return;
        }
    }

    memset(&targetRect, 0, sizeof(targetRect));
}

/**
 * Report decoder is currently decoding frames.
 * User should call this function before sending command to decode the
 * next frame.
 */
bool ImageDecoder::IsBusy(void) {
    return (decodeState == DecodeState::BUSY ? true : false);
}
/*
 * DecodeNextImage():  start decode the next image or frame.
 */
bool ImageDecoder::DecodeNextImage(void) {

    std::unique_lock<std::mutex> decodeLock(decodeMutex);

    if(IsBusy())
        return false;

    int status;
    if(!curImgFile) {
        curImgFile = fileNames.front();
        fileNames.pop();
        // Open the asset with the give name from the APK's assets folder.
        imgAsset =
                AAssetManager_open(assetMgr, curImgFile, AASSET_MODE_BUFFER);
        ASSERT(imgAsset, "%s does not exist in %s", curImgFile,
               __FUNCTION__);

        // Create an AImageDecoder from the given AAsset
        decoder = nullptr;
        status = AImageDecoder_createFromAAsset(imgAsset, &decoder);
        ASSERT(ANDROID_IMAGE_DECODER_SUCCESS == status,
               "Failed to create ImageDecoder for %s with the error of %s",
               curImgFile, AImageDecoder_resultToString(status));

        status = AImageDecoder_setAndroidBitmapFormat(
                decoder, GetDecodingBitmapFormat());
        ASSERT(ANDROID_IMAGE_DECODER_SUCCESS == status,
               "Failed to request decoding output format %s(format: %d)",
               curImgFile, bufInfo.format);

        status = AImageDecoder_setUnpremultipliedRequired(decoder, false);
        ASSERT(ANDROID_IMAGE_DECODER_SUCCESS == status,
               "Failed to use pre-multiply alpha for %s", curImgFile);

        // scale the image to the max possible size to fit into display window
        // but keep the image's aspect ratio.
        headerInfo =AImageDecoder_getHeaderInfo(decoder);
        ASSERT(headerInfo != nullptr, "Failed to get ImageHeaderInfo for %s",
               curImgFile);

        ARect imgRect = {
                .left = 0, .top = 0,
                .right = AImageDecoderHeaderInfo_getWidth(headerInfo),
                .bottom = AImageDecoderHeaderInfo_getHeight(headerInfo)
        };
        ARect dstRect = {
                .left =0, .top = 0,
                .right = bufInfo.width,
                .bottom =bufInfo.height
        };
        ImageScaleDescriptor scalerInfo(dstRect, imgRect);
        rotationAngle = scalerInfo.getScaleRectInfo(&targetRect);
        status = AImageDecoder_setTargetSize(decoder, targetRect.right - targetRect.left,
            targetRect.bottom - targetRect.top);
        ASSERT(ANDROID_IMAGE_DECODER_SUCCESS == status,
               "Failed to set target rect (%d, %d), error:%s", bufInfo.width, bufInfo.height,
               AImageDecoder_resultToString(status));

        // scRGB/sRGB/SCRGB_LINEAR are okay for this sample
        ADataSpace dataSpace =
                static_cast<ADataSpace>(AImageDecoderHeaderInfo_getDataSpace(headerInfo));
        if (dataSpace != ADATASPACE_SCRGB && dataSpace != ADATASPACE_SRGB &&
            dataSpace != ADATASPACE_SCRGB_LINEAR) {
            status = AImageDecoder_setDataSpace(decoder, ADATASPACE_SRGB);
            ASSERT(ANDROID_IMAGE_DECODER_SUCCESS == status,
                   "Failed to set SRGB color space for %s", curImgFile);
        }

        decodedImageStride = AImageDecoder_getMinimumStride(decoder);

        decodedImageBits.resize(bufInfo.height * decodedImageStride);
    }

    frameDuration =  kDEFAULT_DURATION;
    if (!frameInfo) {
        // create frameInfo for each image file, delete it when that file decoding is completed.
        // for animated image files, frameInfo will be created once and reused for the whole file.
        frameInfo = AImageDecoderFrameInfo_create();
        ASSERT(frameInfo, "Failed to create FrameInfo object for %s", curImgFile);
    }
    if (AImageDecoder_isAnimated(decoder) &&
        ANDROID_IMAGE_DECODER_SUCCESS == AImageDecoder_getFrameInfo(decoder, frameInfo)) {
        frameDuration = AImageDecoderFrameInfo_getDuration(frameInfo);
    }

    // Create and start decoding thread: the thread is detached right after creation,
    // hence it will run to completion.
    decodeState = DecodeState::BUSY;
    std::thread decodingThread([&] () {
        std::unique_lock<std::mutex> threadLock(decodeMutex);
        int status = AImageDecoder_decodeImage(decoder,
                                               decodedImageBits.data(),
                                               decodedImageStride,
                                               decodedImageBits.size());
        ASSERT(status == ANDROID_IMAGE_DECODER_SUCCESS, "Failed to decode image %s",
               curImgFile);

        if(!AImageDecoder_isAnimated(decoder) ||
           ANDROID_IMAGE_DECODER_SUCCESS != AImageDecoder_advanceFrame(decoder)) {
            if (frameInfo) {
                AImageDecoderFrameInfo_delete(frameInfo);
                frameInfo = nullptr;
            }

            AImageDecoder_delete(decoder);
            decoder = nullptr;
            AAsset_close(imgAsset);
            imgAsset = nullptr;
            fileNames.push(curImgFile);
            curImgFile = nullptr;
        }
        decodeState = DecodeState::COMPLETED;
    });
    decodingThread.detach();
    return true;
}

/**
 * Report the decoded frame info.
 * param frame - pointer to data structure for decoded frame info
 * return bool:
 *   true - image successfully copied into frame
 *   false - image is not ready (not decoded yet)
 */
bool ImageDecoder::GetDecodedFrame(DecodeFrameDescriptor* _Nonnull frame) {
    if(decodeState == DecodeState::COMPLETED) {
        frame->bits = decodedImageBits.data();
        frame->width = targetRect.right - targetRect.left;
        frame->height = targetRect.bottom - targetRect.top;
        frame->stride = decodedImageStride;
        frame->rotation = rotationAngle;
        frame->displayDuration = frameDuration;

        return true;
    }
    return false;
}

/** GetDecodingBitmapFormat():
 *     translate the required image format to image decoder's format
 */
AndroidBitmapFormat ImageDecoder::GetDecodingBitmapFormat() {
    switch (bufInfo.format) {
        case SurfaceFormat::SURFACE_FORMAT_RGBA_8888:
            return ANDROID_BITMAP_FORMAT_RGBA_8888;
        case SurfaceFormat::SURFACE_FORMAT_RGB_565:
            return ANDROID_BITMAP_FORMAT_RGB_565;
        default:
            LOGE("Unsupported display buffer format (%d) in %s", bufInfo.format, __FUNCTION__);
        }
    return ANDROID_BITMAP_FORMAT_NONE;
}

/**
 * Destructor to wait for outstanding decoding finish before
 * tearing down.
 */
ImageDecoder::~ImageDecoder() {
    // make sure the decoding thread is finished
    std::lock_guard<std::mutex> lock(decodeMutex);
}

