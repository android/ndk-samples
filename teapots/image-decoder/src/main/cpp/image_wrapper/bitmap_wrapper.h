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
 *  This file is a direct refactor from Android NDK-r21b's header file android/bitmap.h
 *  For detailed documentation, please download the above NDK from 
 *        https://developer.android.com/ndk/downloads
 *  
 */

#ifndef ANDROID_BITMAP_WRAPPER_H
#define ANDROID_BITMAP_WRAPPER_H

#include <stdbool.h>
#include <stdint.h>
#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif

/** AndroidBitmap functions result code. */
enum {
    /** Operation was successful. */
    ANDROID_BITMAP_RESULT_SUCCESS           = 0,
    /** Bad parameter. */
    ANDROID_BITMAP_RESULT_BAD_PARAMETER     = -1,
    /** JNI exception occured. */
    ANDROID_BITMAP_RESULT_JNI_EXCEPTION     = -2,
    /** Allocation failed. */
    ANDROID_BITMAP_RESULT_ALLOCATION_FAILED = -3,
};

/** Backward compatibility: this macro used to be misspelled. */
#define ANDROID_BITMAP_RESUT_SUCCESS ANDROID_BITMAP_RESULT_SUCCESS

/** Bitmap pixel format. */
enum AndroidBitmapFormat {
    /** No format. */
    ANDROID_BITMAP_FORMAT_NONE      = 0,
    /** Red: 8 bits, Green: 8 bits, Blue: 8 bits, Alpha: 8 bits. **/
    ANDROID_BITMAP_FORMAT_RGBA_8888 = 1,
    /** Red: 5 bits, Green: 6 bits, Blue: 5 bits. **/
    ANDROID_BITMAP_FORMAT_RGB_565   = 4,
    /** Deprecated in API level 13. Because of the poor quality of this configuration, it is advised to use ARGB_8888 instead. **/
    ANDROID_BITMAP_FORMAT_RGBA_4444 = 7,
    /** Alpha: 8 bits. */
    ANDROID_BITMAP_FORMAT_A_8       = 8,
    /** Each component is stored as a half float. **/
    ANDROID_BITMAP_FORMAT_RGBA_F16  = 9,
};

/** Bitmap alpha format */
enum {
    /** Pixel components are premultiplied by alpha. */
    ANDROID_BITMAP_FLAGS_ALPHA_PREMUL   = 0,
    /** Pixels are opaque. */
    ANDROID_BITMAP_FLAGS_ALPHA_OPAQUE   = 1,
    /** Pixel components are independent of alpha. */
    ANDROID_BITMAP_FLAGS_ALPHA_UNPREMUL = 2,
    /** Bit mask for AndroidBitmapFormat.flags to isolate the alpha. */
    ANDROID_BITMAP_FLAGS_ALPHA_MASK     = 0x3,
    /** Shift for AndroidBitmapFormat.flags to isolate the alpha. */
    ANDROID_BITMAP_FLAGS_ALPHA_SHIFT    = 0,
};

enum {
    /** If this bit is set in AndroidBitmapInfo.flags, the Bitmap uses the
      * HARDWARE Config, and its AHardwareBuffer can be retrieved via
      * AndroidBitmap_getHardwareBuffer.
      */
    ANDROID_BITMAP_FLAGS_IS_HARDWARE = 1 << 31,
};

/** Bitmap info, see AndroidBitmap_getInfo(). */
typedef struct {
    /** The bitmap width in pixels. */
    uint32_t    width;
    /** The bitmap height in pixels. */
    uint32_t    height;
    /** The number of byte per row. */
    uint32_t    stride;
    /** The bitmap pixel format. See {@link AndroidBitmapFormat} */
    int32_t     format;
    /** Two bits are used to encode alpha. Use ANDROID_BITMAP_FLAGS_ALPHA_MASK
      * and ANDROID_BITMAP_FLAGS_ALPHA_SHIFT to retrieve them. One bit is used
      * to encode whether the Bitmap uses the HARDWARE Config. Use
      * ANDROID_BITMAP_FLAGS_IS_HARDWARE to know.*/
    uint32_t    flags;
} AndroidBitmapInfo;


typedef int (*PFN_AndroidBitmap_getInfo)(JNIEnv* env, jobject jbitmap,
                          AndroidBitmapInfo* info);
extern PFN_AndroidBitmap_getInfo AndroidBitmap_getInfo;

typedef int32_t (*PFN_AndroidBitmap_getDataSpace)(JNIEnv* env, jobject jbitmap);
extern PFN_AndroidBitmap_getDataSpace AndroidBitmap_getDataSpace;

typedef int (*PFN_AndroidBitmap_lockPixels)(JNIEnv* env, jobject jbitmap, void** addrPtr);
extern PFN_AndroidBitmap_lockPixels AndroidBitmap_lockPixels;

typedef int (*PFN_AndroidBitmap_unlockPixels)(JNIEnv* env, jobject jbitmap);
extern PFN_AndroidBitmap_unlockPixels AndroidBitmap_unlockPixels;

enum AndroidBitmapCompressFormat {
    ANDROID_BITMAP_COMPRESS_FORMAT_JPEG = 0,
    ANDROID_BITMAP_COMPRESS_FORMAT_PNG = 1,
    ANDROID_BITMAP_COMPRESS_FORMAT_WEBP_LOSSY = 3,
    ANDROID_BITMAP_COMPRESS_FORMAT_WEBP_LOSSLESS = 4,
};
typedef enum AndroidBitmapFormat AndroidBitmapFormat;

typedef bool (*AndroidBitmap_compress_write_fn)(void* userContext,
                                                const void* data,
                                                size_t size);

typedef int (*PFN_AndroidBitmap_compress)(const AndroidBitmapInfo* info,
                           int32_t dataspace,
                           const void* pixels,
                           int32_t format, int32_t quality,
                           void* userContext,
                           AndroidBitmap_compress_write_fn fn);
extern PFN_AndroidBitmap_compress AndroidBitmap_compress;

typedef struct AHardwareBuffer AHardwareBuffer;
typedef int (*PFN_AndroidBitmap_getHardwareBuffer)(JNIEnv* env, jobject bitmap,
        AHardwareBuffer** outBuffer);
extern PFN_AndroidBitmap_getHardwareBuffer AndroidBitmap_getHardwareBuffer;

/**
 * Temporary function functions to load lib at run time and pull out the APIs.
 * This file is to be deleted after the next NDK release ( around Android DP2 )
 */
int  InitBitmap(void);

#ifdef __cplusplus
}
#endif

#endif

/** @} */
