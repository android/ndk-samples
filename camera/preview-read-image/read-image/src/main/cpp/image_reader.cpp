/*
 * Copyright (C) 2017 The Android Open Source Project
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

#include "image_reader.h"
#include "android_native_app_glue.h"
#include "utils/native_debug.h"

/*
 * MAX_BUF_COUNT:
 *   Max buffers in this ImageReader.
 */
#define MAX_BUF_COUNT 8

/*
 * ImageReader Listener: called by Camera for every frame captured
 * We pass the event to ImageReader class, so it could do some housekeeping about
 * the loaded queue. For example, we could keep a counter to track how many
 * buffers are full and idle in the queue. If camera almost has no buffer to capture
 * we could release ( skip ) some frames by AImageReader_getNextImage() and
 * AImageReader_delete().
 */
void OnImageCallback(void* ctx, AImageReader* reader) {
    reinterpret_cast<ImageReader*>(ctx)->ImageCallback(reader);
}

/*
 * Constructor
 */
ImageReader::ImageReader(ImageResolution* res) :
    reader_(nullptr), imageWatermark_(0), presentRotation_(0) {

    media_status_t status = AImageReader_new(res->width, res->height,
                                             AIMAGE_FORMAT_YUV_420_888,
                                             MAX_BUF_COUNT, &reader_);
    ASSERT(reader_ && status == AMEDIA_OK, "Failed to create AImageReader");

    AImageReader_ImageListener listener {
        .context = this,
        .onImageAvailable = OnImageCallback,
    };
    AImageReader_setImageListener(reader_, &listener);
}

ImageReader::~ImageReader() {
    ASSERT(reader_, "NULL Pointer to %s", __FUNCTION__);
    AImageReader_delete(reader_);
}

void ImageReader::ImageCallback(AImageReader* reader) {
    if (++imageWatermark_ >= (MAX_BUF_COUNT - 1)) {
        // We could skip half of the frame so next frame
        // would pick up the middle of the frame, and let capture has buffer to continue
        // AImage_delete(GetNextImage());
        // --imageWatermark_;
        LOGI("Buffers are full, Display is behind");
    }
}

ANativeWindow* ImageReader::GetNativeWindow(void) {
    if (!reader_) return nullptr;
    ANativeWindow* nativeWindow;
    media_status_t status = AImageReader_getWindow(reader_, &nativeWindow);
    ASSERT(status == AMEDIA_OK, "Could not get ANativeWindow");

    return nativeWindow;
}

/*
 * GetLastImage()
 *   Retrieve the newest image inside ImageReader's bufferQueue, drop all BUT the last
 *   one to convert and present to display
 */
AImage* ImageReader::GetLastImage(void) {
    AImage* image;
    AImage* preImage = NULL;
    media_status_t status = AImageReader_acquireNextImage(reader_, &image);

    while (status == AMEDIA_OK) {
        if (preImage) {
            AImage_delete(preImage);
            --imageWatermark_;
        }
        preImage = image;
        status = AImageReader_acquireNextImage(reader_, &image);
    }

    return preImage;
}

/*
 * GetNextImage()
 *   Retrieve the next image in ImageReader's bufferQueue, NOT the last image so no image is
 *   skipped
 */
AImage* ImageReader::GetNextImage(void) {
    AImage* image;
    media_status_t status = AImageReader_acquireNextImage(reader_, &image);
    if (status != AMEDIA_OK) {
        //        LOGI("NO IMAGE AVAILABLE");
        return nullptr;
    }
    return image;
}

/*
 * Helper function for YUV_420 to RGB conversion. Courtesy of Tensorflow ImageClassifier Sample:
 * https://github.com/tensorflow/tensorflow/blob/master/tensorflow/examples/android/jni/yuv2rgb.cc
 * The difference is that here we have to swap UV plane when calling it.
 */
#ifndef MAX
#define MAX(a, b) ({__typeof__(a) _a = (a); __typeof__(b) _b = (b); _a > _b ? _a : _b; })
#define MIN(a, b) ({__typeof__(a) _a = (a); __typeof__(b) _b = (b); _a < _b ? _a : _b; })
#endif

// This value is 2 ^ 18 - 1, and is used to clamp the RGB values before their ranges
// are normalized to eight bits.
static const int kMaxChannelValue = 262143;

static inline uint32_t YUV2RGB(int nY, int nU, int nV) {
    nY -= 16;
    nU -= 128;
    nV -= 128;
    if (nY < 0) nY = 0;

    // This is the floating point equivalent. We do the conversion in integer
    // because some Android devices do not have floating point in hardware.
    // nR = (int)(1.164 * nY + 1.596 * nV);
    // nG = (int)(1.164 * nY - 0.813 * nV - 0.391 * nU);
    // nB = (int)(1.164 * nY + 2.018 * nU);

    int nR = (int)(1192 * nY + 1634 * nV);
    int nG = (int)(1192 * nY - 833 * nV - 400 * nU);
    int nB = (int)(1192 * nY + 2066 * nU);

    nR = MIN(kMaxChannelValue, MAX(0, nR));
    nG = MIN(kMaxChannelValue, MAX(0, nG));
    nB = MIN(kMaxChannelValue, MAX(0, nB));

    nR = (nR >> 10) & 0xff;
    nG = (nG >> 10) & 0xff;
    nB = (nB >> 10) & 0xff;

    return 0xff000000 | (nR << 16) | (nG << 8) | nB;
}

/*
 * DisplayImage()
 *   Present camera image to display.
 */
bool ImageReader::DisplayImage(ANativeWindow_Buffer* buf) {

    AImage* image = GetLastImage();
    if (!image) {
        return false;
    }

    --imageWatermark_;

    switch (presentRotation_) {
        case 0:
            PresentImage(buf, image);
            break;
        case 90:
            PresentImage90(buf, image);
            break;
        case 180:
            PresentImage180(buf, image);
            break;
        case 270:
            PresentImage270(buf, image);
            break;
        default:
            ASSERT(0, "NOT recognized display rotation: %d", presentRotation_);
    }

    AImage_delete(image);

    return true;
}

/*
 * PresentImage()
 *   Converting yuv to RGB
 *   No rotation: (x,y) --> (x, y)
 *   Refer to: https://mathbits.com/MathBits/TISection/Geometry/Transformations2.htm
 */
void ImageReader::PresentImage(ANativeWindow_Buffer* buf, AImage* image) {
    int srcFormat = -1;
    AImage_getFormat(image, &srcFormat);
    ASSERT(AIMAGE_FORMAT_YUV_420_888 == srcFormat, "Failed to get format");
    int srcPlanes = 0;
    AImage_getNumberOfPlanes(image, &srcPlanes);
    ASSERT(srcPlanes == 3, "Is not 3 planes");
    AImageCropRect srcRect;
    AImage_getCropRect(image, &srcRect);

    int yStride, uvStride;
    uint8_t *yPixel, *uPixel, *vPixel;
    int32_t yLen, uLen, vLen;
    AImage_getPlaneRowStride(image, 0, &yStride);
    AImage_getPlaneRowStride(image, 1, &uvStride);
    AImage_getPlaneData(image, 0, &yPixel, &yLen);
    AImage_getPlaneData(image, 1, &vPixel, &vLen);
    AImage_getPlaneData(image, 2, &uPixel, &uLen);
    int32_t uvPixelStride;
    AImage_getPlanePixelStride(image, 1, &uvPixelStride);

    ASSERT(buf->format == WINDOW_FORMAT_RGBX_8888, "Not asked buffer format");

    int height = MIN(buf->height, (srcRect.bottom - srcRect.top));
    int width = MIN(buf->width, (srcRect.right - srcRect.left));

    uint32_t *out = static_cast<uint32_t *>(buf->bits);
    for (int y = 0; y < height; y++) {
        const uint8_t *pY = yPixel + yStride * (y + srcRect.top) + srcRect.left;

        int uv_row_start = uvStride * ((y + srcRect.top) >> 1);
        const uint8_t *pU = uPixel + uv_row_start + (srcRect.left >> 1);
        const uint8_t *pV = vPixel + uv_row_start + (srcRect.left >> 1);

        for (int x = 0; x < width; x++) {
            const int uv_offset = (x >> 1) * uvPixelStride;
            out[x] = YUV2RGB(pY[x], pU[uv_offset], pV[uv_offset]);
        }
        out += buf->stride;
    }
}

/*
 * PresentImage90()
 *   Converting YUV to RGB
 *   Rotation image anti-clockwise 90 degree -- (x, y) --> (-y, x)
 */
void ImageReader::PresentImage90(ANativeWindow_Buffer* buf, AImage* image) {
    int srcFormat = -1;
    AImage_getFormat(image, &srcFormat);
    ASSERT(AIMAGE_FORMAT_YUV_420_888 == srcFormat, "Failed to get format");
    int srcPlanes = 0;
    AImage_getNumberOfPlanes(image, &srcPlanes);
    ASSERT(srcPlanes == 3, "Is not 3 planes");
    AImageCropRect srcRect;
    AImage_getCropRect(image, &srcRect);

    int yStride, uvStride;
    uint8_t *yPixel, *uPixel, *vPixel;
    int32_t yLen, uLen, vLen;
    AImage_getPlaneRowStride(image, 0, &yStride);
    AImage_getPlaneRowStride(image, 1, &uvStride);
    AImage_getPlaneData(image, 0, &yPixel, &yLen);
    AImage_getPlaneData(image, 1, &vPixel, &vLen);
    AImage_getPlaneData(image, 2, &uPixel, &uLen);
    int32_t uvPixelStride;
    AImage_getPlanePixelStride(image, 1, &uvPixelStride);

    ASSERT(buf->format == WINDOW_FORMAT_RGBX_8888, "Not asked buffer format");

    int32_t srcHeight = MIN(buf->width, (srcRect.bottom - srcRect.top));
    int32_t srcWidth = MIN(buf->height, (srcRect.right - srcRect.left));

    uint32_t *out = static_cast<uint32_t *>(buf->bits);
    out += srcHeight - 1;
    for (int y = 0; y < srcHeight; y++) {
        const uint8_t *pY = yPixel + yStride * (y + srcRect.top) + srcRect.left;

        int uv_row_start = uvStride * ((y + srcRect.top) >> 1);
        const uint8_t *pU = uPixel + uv_row_start + (srcRect.left >> 1);
        const uint8_t *pV = vPixel + uv_row_start + (srcRect.left >> 1);

        for (int x = 0; x < srcWidth; x++) {
            const int uv_offset = (x >> 1) * uvPixelStride;
            // [x, y]--> [-y, x]
            out[x * buf->stride] = YUV2RGB(pY[x], pU[uv_offset], pV[uv_offset]);
        }
        out -= 1;  // move to the next column
    }
}

/*
 * PresentImage180()
 *   Converting yuv to RGB
 *   Rotate image 180 degree: (x, y) --> (-x, -y)
 */
void ImageReader::PresentImage180(ANativeWindow_Buffer* buf, AImage* image) {
    int srcFormat = -1;
    AImage_getFormat(image, &srcFormat);
    ASSERT(AIMAGE_FORMAT_YUV_420_888 == srcFormat, "Failed to get format");
    int srcPlanes = 0;
    AImage_getNumberOfPlanes(image, &srcPlanes);
    ASSERT(srcPlanes == 3, "Is not 3 planes");
    AImageCropRect srcRect;
    AImage_getCropRect(image, &srcRect);

    int yStride, uvStride;
    uint8_t *yPixel, *uPixel, *vPixel;
    int32_t yLen, uLen, vLen;
    AImage_getPlaneRowStride(image, 0, &yStride);
    AImage_getPlaneRowStride(image, 1, &uvStride);
    AImage_getPlaneData(image, 0, &yPixel, &yLen);
    AImage_getPlaneData(image, 1, &vPixel, &vLen);
    AImage_getPlaneData(image, 2, &uPixel, &uLen);
    int32_t uvPixelStride;
    AImage_getPlanePixelStride(image, 1, &uvPixelStride);

    ASSERT(buf->format == WINDOW_FORMAT_RGBX_8888, "Not asked buffer format");

    int height = MIN(buf->height, (srcRect.bottom - srcRect.top));
    int width = MIN(buf->width, (srcRect.right - srcRect.left));

    uint32_t *out = static_cast<uint32_t *>(buf->bits);
    out += (height - 1) * buf->stride;
    for (int y = 0; y < height; y++) {
        const uint8_t *pY = yPixel + yStride * (y + srcRect.top) + srcRect.left;

        int uv_row_start = uvStride * ((y + srcRect.top) >> 1);
        const uint8_t *pU = uPixel + uv_row_start + (srcRect.left >> 1);
        const uint8_t *pV = vPixel + uv_row_start + (srcRect.left >> 1);

        for (int x = 0; x < width; x++) {
            const int uv_offset = (x >> 1) * uvPixelStride;
            // mirror image since we are using from camera
            out[width - 1 - x] = YUV2RGB(pY[x], pU[uv_offset], pV[uv_offset]);
        }
        out -= buf->stride;
    }
}

/*
 * PresentImage270()
 *   Converting image from YUV to RGB
 *   Rotate Image counter-clockwise 270 degreee: (x, y) --> (y, -x)
 */
void ImageReader::PresentImage270(ANativeWindow_Buffer* buf, AImage* image) {
    int srcFormat = -1;
    AImage_getFormat(image, &srcFormat);
    ASSERT(AIMAGE_FORMAT_YUV_420_888 == srcFormat, "Failed to get format");
    int srcPlanes = 0;
    AImage_getNumberOfPlanes(image, &srcPlanes);
    ASSERT(srcPlanes == 3, "Is not 3 planes");
    AImageCropRect srcRect;
    AImage_getCropRect(image, &srcRect);

    int yStride, uvStride;
    uint8_t *yPixel, *uPixel, *vPixel;
    int32_t yLen, uLen, vLen;
    AImage_getPlaneRowStride(image, 0, &yStride);
    AImage_getPlaneRowStride(image, 1, &uvStride);
    AImage_getPlaneData(image, 0, &yPixel, &yLen);
    AImage_getPlaneData(image, 1, &vPixel, &vLen);
    AImage_getPlaneData(image, 2, &uPixel, &uLen);
    int32_t uvPixelStride;
    AImage_getPlanePixelStride(image, 1, &uvPixelStride);

    ASSERT(buf->format == WINDOW_FORMAT_RGBX_8888, "Not asked buffer format");

    int height = MIN(buf->width, (srcRect.bottom - srcRect.top));
    int width = MIN(buf->height, (srcRect.right - srcRect.left));

    uint32_t *out = static_cast<uint32_t *>(buf->bits);
    out += height - 1;
    for (int y = 0; y < height; y++) {
        const uint8_t *pY = yPixel + yStride * (y + srcRect.top) + srcRect.left;

        int uv_row_start = uvStride * ((y + srcRect.top) >> 1);
        const uint8_t *pU = uPixel + uv_row_start + (srcRect.left >> 1);
        const uint8_t *pV = vPixel + uv_row_start + (srcRect.left >> 1);

        for (int x = 0; x < width; x++) {
            const int uv_offset = (x >> 1) * uvPixelStride;
            out[(width - 1 - x) * buf->stride] = YUV2RGB(pY[x], pU[uv_offset], pV[uv_offset]);
        }
        out -= 1;  // move to the next column
    }
}
void ImageReader::SetPresentRotation(int32_t angle) {
    presentRotation_ = angle;
}