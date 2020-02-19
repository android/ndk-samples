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

/**
 *     ImageDecoder_wrapper.h
 *  This file is a direct refactor from Android NDK-r21b's header file android/ImageDecoder.h
 *  For detailed documentation, please download the above NDK from 
 *        https://developer.android.com/ndk/downloads
 */

/**
 * @file imageDecoder_wrapper.h
 */

#ifndef ANDROID_IMAGE_DECODER_WRAPPER_H
#define ANDROID_IMAGE_DECODER_WRAPPER_H

#include "bitmap_wrapper.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct AAsset;
struct ARect;


/** AImageDecoder functions result code. */
enum {
    // Decoding was successful and complete.
    ANDROID_IMAGE_DECODER_SUCCESS = 0,
    // The input was incomplete. In decodeImage, this means a partial
    // image was decoded. Undecoded lines are all zeroes.
    // In AImageDecoder_create*, no AImageDecoder was created.
    ANDROID_IMAGE_DECODER_INCOMPLETE = -1,
    // The input contained an error after decoding some lines. Similar to
    // INCOMPLETE, above.
    ANDROID_IMAGE_DECODER_ERROR = -2,
    // Could not convert, e.g. attempting to decode an image with
    // alpha to an opaque format.
    ANDROID_IMAGE_DECODER_INVALID_CONVERSION = -3,
    // The scale is invalid. It may have overflowed, or it may be incompatible
    // with the current alpha setting.
    ANDROID_IMAGE_DECODER_INVALID_SCALE = -4,
    // Some other parameter was bad (e.g. pixels)
    ANDROID_IMAGE_DECODER_BAD_PARAMETER = -5,
    // Input was invalid i.e. broken before decoding any pixels.
    ANDROID_IMAGE_DECODER_INVALID_INPUT = -6,
    // A seek was required, and failed.
    ANDROID_IMAGE_DECODER_SEEK_ERROR = -7,
    // Some other error (e.g. OOM)
    ANDROID_IMAGE_DECODER_INTERNAL_ERROR = -8,
    // We did not recognize the format
    ANDROID_IMAGE_DECODER_UNSUPPORTED_FORMAT = -9
};

struct AImageDecoder;

/**
 * Opaque handle for decoding images.
 *
 * Create using one of the following:
 * - {@link AImageDecoder_createFromAAsset}
 * - {@link AImageDecoder_createFromFd}
 * - {@link AImageDecoder_createFromBuffer}
 */
typedef struct AImageDecoder AImageDecoder;

typedef int (*PFN_AImageDecoder_createFromAAsset)(struct AAsset* asset, AImageDecoder** outDecoder);
extern PFN_AImageDecoder_createFromAAsset AImageDecoder_createFromAAsset;

typedef int (*PFN_AImageDecoder_createFromFd)(int fd, AImageDecoder** outDecoder);
extern PFN_AImageDecoder_createFromFd AImageDecoder_createFromFd;


typedef int (*PFN_AImageDecoder_createFromBuffer)(const void* buffer, size_t length,
                                   AImageDecoder** outDecoder);

extern PFN_AImageDecoder_createFromBuffer AImageDecoder_createFromBuffer;

typedef void (*PFN_AImageDecoder_delete)(AImageDecoder* decoder);
extern PFN_AImageDecoder_delete AImageDecoder_delete;


typedef int (*PFN_AImageDecoder_setAndroidBitmapFormat)(AImageDecoder*,
        int32_t format);
extern  PFN_AImageDecoder_setAndroidBitmapFormat AImageDecoder_setAndroidBitmapFormat;

typedef int (*PFN_AImageDecoder_setUnpremultipliedRequired)(AImageDecoder*, bool required);
extern PFN_AImageDecoder_setUnpremultipliedRequired AImageDecoder_setUnpremultipliedRequired;

typedef int (*PFN_AImageDecoder_setDataSpace)(AImageDecoder*, int32_t dataspace);
extern PFN_AImageDecoder_setDataSpace AImageDecoder_setDataSpace;

typedef int (*PFN_AImageDecoder_setTargetSize)(AImageDecoder*, int width, int height);
extern PFN_AImageDecoder_setTargetSize AImageDecoder_setTargetSize;

typedef int (*PFN_AImageDecoder_computeSampledSize)(const AImageDecoder*, int sampleSize,
                                     int* width, int* height);
extern PFN_AImageDecoder_computeSampledSize AImageDecoder_computeSampledSize;


typedef int (*PFN_AImageDecoder_setCrop)(AImageDecoder*, struct ARect crop);
extern  PFN_AImageDecoder_setCrop AImageDecoder_setCrop;



/**
 * Opaque handle for reading header info.
 */
struct AImageDecoderHeaderInfo;
typedef struct AImageDecoderHeaderInfo AImageDecoderHeaderInfo;

typedef const AImageDecoderHeaderInfo* (*PFN_AImageDecoder_getHeaderInfo)(
        const AImageDecoder*);
extern PFN_AImageDecoder_getHeaderInfo AImageDecoder_getHeaderInfo;

typedef int32_t (*PFN_AImageDecoderHeaderInfo_getWidth)(const AImageDecoderHeaderInfo*);
extern  PFN_AImageDecoderHeaderInfo_getWidth AImageDecoderHeaderInfo_getWidth;

typedef int32_t (*PFN_AImageDecoderHeaderInfo_getHeight)(const AImageDecoderHeaderInfo*);
extern PFN_AImageDecoderHeaderInfo_getHeight AImageDecoderHeaderInfo_getHeight;

typedef const char* (*PFN_AImageDecoderHeaderInfo_getMimeType)(
        const AImageDecoderHeaderInfo*);
extern PFN_AImageDecoderHeaderInfo_getMimeType AImageDecoderHeaderInfo_getMimeType;

typedef bool (*PFN_AImageDecoderHeaderInfo_isAnimated)(
        const AImageDecoderHeaderInfo*);
extern PFN_AImageDecoderHeaderInfo_isAnimated AImageDecoderHeaderInfo_isAnimated;

typedef AndroidBitmapFormat (*PFN_AImageDecoderHeaderInfo_getAndroidBitmapFormat)(
        const AImageDecoderHeaderInfo*);
extern PFN_AImageDecoderHeaderInfo_getAndroidBitmapFormat AImageDecoderHeaderInfo_getAndroidBitmapFormat;

typedef int (*PFN_AImageDecoderHeaderInfo_getAlphaFlags)(
        const AImageDecoderHeaderInfo*);
extern PFN_AImageDecoderHeaderInfo_getAlphaFlags AImageDecoderHeaderInfo_getAlphaFlags;

typedef int32_t (*PFN_AImageDecoderHeaderInfo_getDataSpace)(
        const AImageDecoderHeaderInfo*);
extern PFN_AImageDecoderHeaderInfo_getDataSpace AImageDecoderHeaderInfo_getDataSpace;

typedef size_t (*PFN_AImageDecoder_getMinimumStride)(AImageDecoder*);
extern PFN_AImageDecoder_getMinimumStride AImageDecoder_getMinimumStride __INTRODUCED_IN(29);

typedef int (*PFN_AImageDecoder_decodeImage)(AImageDecoder* decoder,
                              void* pixels, size_t stride,
                              size_t size);
extern PFN_AImageDecoder_decodeImage AImageDecoder_decodeImage;


/**
 * Temporary function functions to load lib at run time and pull out the APIs.
 * This file is to be deleted after the next NDK release ( around Android DP2 )
 */
int  InitImageDecoder(void);

#ifdef __cplusplus
}
#endif

#endif // ANDROID_IMAGE_DECODER_H

/** @} */
