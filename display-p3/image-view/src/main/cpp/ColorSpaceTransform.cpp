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
 *
 */
#include <cmath>
#include <cstdlib>
#include <vector>
#include "android_debug.h"
#include "ColorSpaceTransform.h"

#define EPSILON  0.000001f
#define HAS_GAMMA(x) (std::abs(x) > EPSILON && std::abs((x) - 1.0f) > EPSILON)
#define CLIP_COLOR(color, max) ((color > max) ? max : ((color > 0) ? color : 0))

/*
 * CreateGammaEncodeTable():
 *     sRGB =
 *        12.92 * LinearRGB            0 < LinearRGB < 0.0031308
 *        1.055 * power(LinearRGB, gamma)-0.055 0.0031308 <= LinarRGB <= 1.0f
 */
static void CreateGammaEncodeTable(float gamma, std::vector<uint8_t>& table) {
  ASSERT(gamma < 1.0f, "Wrong Gamma (%f) for encoding", gamma);
  uint32_t maxPixeli = ( 1<<8 ) - 1;
  float maxPixelf = static_cast<float>(maxPixeli);

  uint32_t maxLinearVal = static_cast<uint32_t>(0.0031308f * maxPixeli);

  table.resize(0);
  for(uint32_t idx = 0; idx < maxLinearVal; idx++) {
    double val = idx * 12.92 + .5f;
    table.push_back(static_cast<uint8_t>(val));
  }

  for (uint32_t idx = maxLinearVal; idx <= maxPixeli; idx++) {
    double val = (1.055f * pow(idx / maxPixelf, gamma) - 0.055f);
    val = val * maxPixeli + 0.5f;
    table.push_back(static_cast<uint8_t>(CLIP_COLOR(val, maxPixelf)));
  }
}
/*
 * CreateGammaEncodeTable()
 *    Retrieve linear RGB data
 *    Linear =  sRGB / 12.92    0 <= sRGB < 0.04045
 *              pow((sRGB + 0.055)/1.055, gamma)
 */
static void CreateGammaDecodeTable(float gamma, std::vector<uint8_t>&table) {
  ASSERT(gamma > 1.0, "Wrong Gamma(%f) for decoding", gamma);
  uint32_t maxPixeli = ( 1<<8 ) - 1;
  float maxPixelf = static_cast<float>(maxPixeli);

  uint32_t maxLinearVal = static_cast<uint32_t>(0.04045 * maxPixeli);
  for(uint32_t idx = 0; idx < maxLinearVal; idx++) {
    double val = idx / 12.92 + .5f;
    table.push_back(static_cast<uint8_t>(val));
  }

  for (uint32_t idx = maxLinearVal; idx <= maxPixeli; idx++) {
    double val;
    val = (idx / maxPixelf + 0.055f) / 1.055f;
    val = pow(val, gamma) * maxPixeli + 0.5f;
    table.push_back(static_cast<uint8_t>(CLIP_COLOR(val, maxPixelf)));
  }
}

/*
 * ApplyGamma()
 *    Perform gamma lookup for RGBA8888 format
 */
static bool ApplyGamma(void* dst, void* src, uint32_t w, uint32_t h,
                std::vector<uint8_t>& gammaTable) {
  if(!src || !dst || gammaTable.empty()) {
    LOGE("Invalid Input to %s, dst(%p),src(%p)",
         __FUNCTION__, dst, src);
    return false;
  }
  uint8_t* imgSrc = static_cast<uint8_t*>(src);
  uint8_t* imgDst = static_cast<uint8_t*>(dst);
  for (uint32_t r = 0; r < w; r++) {
    for(uint32_t c = 0; c < h; c++) {
      *imgDst++ = gammaTable[*imgSrc++];
      *imgDst++ = gammaTable[*imgSrc++];
      *imgDst++ = gammaTable[*imgSrc++];
      *imgDst++ = *imgSrc++;
    }
  }
  return true;
}

/*
 * GammaDecode(IMAGE_FORMAT& src):
 *    Decode gamma for src image
 *    src.gamma: the gamma used encoding the src image
 * Ouput:
 *     src.buf_: gamma decoded image with gamma value of 1.0f/src.gamma
 */
static bool GammaDecode(IMAGE_FORMAT &src) {
  if (!HAS_GAMMA(src.gamma_)) {
    LOGE("No gamma value to source gamma");
    return true;
  }
  std::vector<uint8_t> gammaTable;
  CreateGammaDecodeTable(1.0f/src.gamma_, gammaTable);

  return ApplyGamma(src.buf_, src.buf_, src.width_, src.height_,
                    gammaTable);
}

static bool GammaDecode(IMAGE_FORMAT &dst, IMAGE_FORMAT &src) {
  if (!HAS_GAMMA(src.gamma_)) {
    LOGE("No gamma value to source gamma");
    return true;
  }
  std::vector<uint8_t> gammaTable;
  CreateGammaDecodeTable(1.0f/src.gamma_, gammaTable);

  return ApplyGamma(dst.buf_, src.buf_, src.width_, src.height_,
                    gammaTable);
}

/*
 * GammaEncode(IMAGE_FORMAT& format dst)
 *    Gamma encode image with the gamma value of dst.gamma_
 */
static bool GammaEncode(IMAGE_FORMAT &dst) {
  if (!HAS_GAMMA(dst.gamma_)) {
    LOGE("No gamma value to dst gamma");
    return true;
  }
  std::vector<uint8_t> gammaTable;
  CreateGammaEncodeTable(dst.gamma_, gammaTable);

  return ApplyGamma(dst.buf_, dst.buf_, dst.width_, dst.height_,
                    gammaTable);
}

/*
 * ApplyTransform8888()
 *    dst = matrix * src
 *    and clamp the result to 0 -- 255
 */
static bool TransformR8G8B8A8(uint8_t* dst, uint8_t *src,
                           uint32_t width, uint32_t height,
                           mathfu::mat3& transMatrix) {
  ASSERT(src && dst, "Wrong image store to %s", __FUNCTION__);

  int32_t m00, m01, m02, m10, m11, m12, m20, m21, m22;
  m00 = static_cast<int32_t>(transMatrix(0, 0) * 1024 + 0.5f),
  m01 = static_cast<int32_t>(transMatrix(0, 1) * 1024 + 0.5f);
  m02 = static_cast<int32_t>(transMatrix(0, 2) * 1024 + 0.5f);
  m10 = static_cast<int32_t>(transMatrix(1, 0) * 1024 + 0.5f);
  m11 = static_cast<int32_t>(transMatrix(1, 1) * 1024 + 0.5f);
  m12 = static_cast<int32_t>(transMatrix(1, 2) * 1024 + 0.5f);
  m20 = static_cast<int32_t>(transMatrix(2, 0) * 1024 + 0.5f);
  m21 = static_cast<int32_t>(transMatrix(2, 1) * 1024 + 0.5f);
  m22 = static_cast<int32_t>(transMatrix(2, 2) * 1024 + 0.5f);
    for (uint32_t row = 0; row < height; row++) {
      for (uint32_t col = 0; col < width; col++) {
        int32_t r, g, b;
        r = (m00 * src[0] + m01 * src[1] + m02 * src[2] + 512) >> 10;
        g = (m10 * src[0] + m11 * src[1] + m12 * src[2] + 512) >> 10;
        b = (m20 * src[0] + m21 * src[1] + m22 * src[2] + 512) >> 10;
        *dst++ = static_cast<uint8_t>(CLIP_COLOR(r, 255));
        *dst++ = static_cast<uint8_t>(CLIP_COLOR(g, 255));
        *dst++ = static_cast<uint8_t>(CLIP_COLOR(b, 255));

        src += 3;          // update src pointer
        *dst++ = *src++;   // copy alpha
      }
    }
  return true;
}

/*
 * Interface Function:
 *     Convert Color Spaces
 */
bool TransformColorSpace(IMAGE_FORMAT &dst, IMAGE_FORMAT& src) {
  if (!src.npm_  || !dst.npm_ || !dst.buf_ || !src.buf_) {
    LOGE("=====Error: Invalid Parameters to TransformColorSpace()");
    return false;
  }

  uint8_t* dstBits = static_cast<uint8_t*>(dst.buf_);
  uint8_t* srcBits = static_cast<uint8_t*>(src.buf_);

  if (HAS_GAMMA(src.gamma_)) {
    GammaDecode(dst, src);
    srcBits = dstBits;
  }

  mathfu::mat3 matrix = *dst.npm_ * (*src.npm_);
  TransformR8G8B8A8(dstBits, srcBits, src.width_, src.height_, matrix);

  if (HAS_GAMMA(dst.gamma_)) {
    GammaEncode(dst);
  }

  return true;
}


/*
 * Default NPMs with white reference points as D65
 * The array sequence should match enum NPM_TYPE definition
    enum NPM_TYPE {
      SRGB_D65 = 0,
      SRGB_D65_INV,
      P3_D65,
      P3_D65_INV,
      TYPE_COUNT
   };
 */
static mathfu::mat3 defaultNPMs[] =  {
    /* SRGB --> XYZ, White reference point D65
     *    Digital Video and HDTV
     *         Algorithms and Interfaces
     *      Charles Poynton
     */
    mathfu::mat3(0.412453f, 0.212671f, 0.019334f,
                 0.357580f, 0.715160f, 0.119193f,
                 0.180423f, 0.072169f, 0.950227f),
    // XYZ --> SRGB white reference point D65
    mathfu::mat3( 3.240479f, -0.969256f, 0.055648f,
                  -1.537150f, 1.875992f, -0.204043f,
                  -0.498535f, 0.041556f, 1.057311f),
    /* P3 --> XYZ, white reference point D65
    *     SMTP EG 432-1:2010, p23
    */
    mathfu::mat3(0.4551698156f, 0.2094916779f, 0.000000000f,
                 0.2771344092f, 0.7215952542f, 0.0470605601f,
                 0.1722826698f, 0.0689130679f, 0.9073553944f),
    /*
     * CIE XYZ --> Display P3, white reference point D65. Source
     *     SMTP EG 432-1:2010, p24
     */
    mathfu::mat3(2.7253940305f, -0.7951680258f, 0.0412418914f,
                -1.0180030062f, 1.6897320548f, -0.0876390192f,
                -0.4401631952f, 0.0226471906f, 1.1009293786f),
};
const mathfu::mat3* GetTransformNPM(NPM_TYPE type) {
  ASSERT(type < NPM_TYPE::TYPE_COUNT, "NPM_TYPE (%d) out of bounds", type);
  return &defaultNPMs[type];
}
