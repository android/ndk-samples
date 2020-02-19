/*
 * Copyright (C) 2020 The Android Open Source Project
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

#include "imagedecoder_wrapper.h"
#include <dlfcn.h>

/**
 * Initialize the AImageDecoder APIs in <android/imagedecoder.h> within
 * Android NDK-r21b. The purpose is to workaround one DP1 issue in
 * Android Gradle Plugin where API Level 30 is not correctly passed into
 * NDK. The issue will be fixed in the next Android Plugin release.
 *
 * return:
 *     0: failed, caller should stop using any bitmap APIs
 *     1: success
 */
int  InitImageDecoder(void) {
    void* libgfx = dlopen("libjnigraphics.so", RTLD_NOW | RTLD_LOCAL);
    if (!libgfx)
        return 0;

#define GET_FUNC(x) (x=(PFN_##x)dlsym(libgfx, #x))
    GET_FUNC(AImageDecoder_createFromAAsset);
    GET_FUNC(AImageDecoder_createFromFd);
    GET_FUNC(AImageDecoder_createFromBuffer);
    GET_FUNC(AImageDecoder_delete);

    GET_FUNC(AImageDecoder_setAndroidBitmapFormat);
    GET_FUNC(AImageDecoder_setUnpremultipliedRequired);
    GET_FUNC(AImageDecoder_setDataSpace);
    GET_FUNC(AImageDecoder_setTargetSize);
    GET_FUNC(AImageDecoder_computeSampledSize);
    GET_FUNC(AImageDecoder_setCrop);
    GET_FUNC(AImageDecoder_getHeaderInfo);
    GET_FUNC(AImageDecoderHeaderInfo_getWidth);
    GET_FUNC(AImageDecoderHeaderInfo_getHeight);

    GET_FUNC(AImageDecoderHeaderInfo_getMimeType);
    GET_FUNC(AImageDecoderHeaderInfo_isAnimated);
    GET_FUNC(AImageDecoderHeaderInfo_getAndroidBitmapFormat);
    GET_FUNC(AImageDecoderHeaderInfo_getAlphaFlags);
    GET_FUNC(AImageDecoderHeaderInfo_getDataSpace);
    GET_FUNC(AImageDecoder_getMinimumStride);

    GET_FUNC(AImageDecoder_decodeImage);
#undef GET_FUNC

    if (AImageDecoder_createFromAAsset && AImageDecoder_createFromFd &&
        AImageDecoder_createFromBuffer && AImageDecoder_delete &&
        AImageDecoder_setAndroidBitmapFormat && AImageDecoder_setUnpremultipliedRequired &&
        AImageDecoder_setDataSpace && AImageDecoder_setTargetSize &&
        AImageDecoder_computeSampledSize && AImageDecoder_setCrop &&
        AImageDecoder_getHeaderInfo && AImageDecoderHeaderInfo_getWidth &&
        AImageDecoderHeaderInfo_getHeight && AImageDecoderHeaderInfo_getMimeType &&
        AImageDecoderHeaderInfo_isAnimated && AImageDecoderHeaderInfo_getAndroidBitmapFormat &&
        AImageDecoderHeaderInfo_getAlphaFlags && AImageDecoderHeaderInfo_getDataSpace &&
        AImageDecoder_getMinimumStride) {
        return 1;
    }

    return 0;
}

PFN_AImageDecoder_createFromAAsset AImageDecoder_createFromAAsset;
PFN_AImageDecoder_createFromFd AImageDecoder_createFromFd;
PFN_AImageDecoder_createFromBuffer AImageDecoder_createFromBuffer;
PFN_AImageDecoder_delete AImageDecoder_delete;
PFN_AImageDecoder_setAndroidBitmapFormat AImageDecoder_setAndroidBitmapFormat;
PFN_AImageDecoder_setUnpremultipliedRequired AImageDecoder_setUnpremultipliedRequired;
PFN_AImageDecoder_setDataSpace AImageDecoder_setDataSpace;
PFN_AImageDecoder_setTargetSize AImageDecoder_setTargetSize;
PFN_AImageDecoder_computeSampledSize AImageDecoder_computeSampledSize;
PFN_AImageDecoder_setCrop AImageDecoder_setCrop;
PFN_AImageDecoder_getHeaderInfo AImageDecoder_getHeaderInfo;
PFN_AImageDecoderHeaderInfo_getWidth AImageDecoderHeaderInfo_getWidth;
PFN_AImageDecoderHeaderInfo_getHeight AImageDecoderHeaderInfo_getHeight;
PFN_AImageDecoderHeaderInfo_getMimeType AImageDecoderHeaderInfo_getMimeType;
PFN_AImageDecoderHeaderInfo_isAnimated AImageDecoderHeaderInfo_isAnimated;
PFN_AImageDecoderHeaderInfo_getAndroidBitmapFormat AImageDecoderHeaderInfo_getAndroidBitmapFormat;
PFN_AImageDecoderHeaderInfo_getAlphaFlags AImageDecoderHeaderInfo_getAlphaFlags;
PFN_AImageDecoderHeaderInfo_getDataSpace AImageDecoderHeaderInfo_getDataSpace;
PFN_AImageDecoder_getMinimumStride AImageDecoder_getMinimumStride;
PFN_AImageDecoder_decodeImage AImageDecoder_decodeImage;

/**
 * Initialize the bitmap api in Android NDK-r21b. The purpose is to workaround
 * one DP1 issue in Android Gradle Plugin where api-30 is not correctly passed into
 * NDK. The issue will be fixed in the next Android Plugin release.
 *
 * return:
 *     0: failed, caller should stop using any bitmap APIs
 *     1: success
 */
int  InitBitmap(void) {
    void* libgfx = dlopen("libjnigraphics.so", RTLD_NOW | RTLD_LOCAL);
    if (!libgfx)
        return 0;

#define GET_FUNC(x) (x=(PFN_##x)dlsym(libgfx, #x))
    GET_FUNC(AndroidBitmap_getInfo);
    GET_FUNC(AndroidBitmap_getDataSpace);
    GET_FUNC(AndroidBitmap_lockPixels);
    GET_FUNC(AndroidBitmap_unlockPixels);
    GET_FUNC(AndroidBitmap_compress);
    GET_FUNC(AndroidBitmap_getHardwareBuffer);
#undef GET_FUNC

    if (AndroidBitmap_getInfo &&
        AndroidBitmap_getDataSpace &&
        AndroidBitmap_lockPixels &&
        AndroidBitmap_unlockPixels &&
        AndroidBitmap_compress &&
        AndroidBitmap_getHardwareBuffer) {
        return 1;
    }

    return 0;
}

PFN_AndroidBitmap_getInfo AndroidBitmap_getInfo;
PFN_AndroidBitmap_getDataSpace AndroidBitmap_getDataSpace;
PFN_AndroidBitmap_lockPixels AndroidBitmap_lockPixels;
PFN_AndroidBitmap_unlockPixels AndroidBitmap_unlockPixels;
PFN_AndroidBitmap_compress AndroidBitmap_compress;
PFN_AndroidBitmap_getHardwareBuffer AndroidBitmap_getHardwareBuffer;