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
#include "common.h"
#include "simple_png.h"

union littleEndianUint32 {
  uint8_t bytes[4];
  uint32_t value;
};
// value, store, offset
#define READ_INT_SWAP(v, s, o)  do {   \
  v.bytes[3] = s[o + 0];   \
  v.bytes[2] = s[o + 1];   \
  v.bytes[1] = s[o + 2];   \
  v.bytes[0] = s[o + 3];   \
} while (0)


// Little endian chunk name
#define  PNG_CHUNCK(c1, c2, c3, c4)  (((c1)<<24) | ((c2)<<16) | ((c3)<<8) | (c4))

/*
 * Parse PNG file header, refer to:
 *    https://www.w3.org/TR/PNG/#11Chunks
 */
PNGHeader::PNGHeader(std::string& name, uint8_t *buf, uint64_t len) :
    name_(name), buf_( buf), length_(len), offset_(0),
    hasChrm_(false), valid_(false) {

  ASSERT(buf_, "PNG header is not initialized");
  NPM_ = mathfu::mat3::Identity();

  // always have a gamma, either from PNG file or our default value
  gamma_ = DEFAULT_IMAGE_GAMMA;
  uint8_t sig[] = {137, 80, 78, 71, 13, 10, 26, 10};

  if (memcmp(buf_, sig, sizeof(sig))) {
    LOGE("==== PNG file %s corrupted", name_.c_str());
    return;
  }
  offset_ += static_cast<uint64_t>(sizeof(sig));

  bool has_sRGB = false;
  bool has_iCCP = false;
  LOGV("=== Parsing File: %s", name_.c_str());
  while (offset_ < length_) {
    littleEndianUint32 len, type;

    /*
     * Read the len word ( 4 bytes ), it is only for data field - no self, not for type
     * not for CRC
     */
    READ_INT_SWAP(len, buf_, offset_);
    offset_ += 4;

    READ_INT_SWAP(type, buf_, offset_);
    offset_ += 4;

    switch (type.value) {
      case PNG_CHUNCK('I', 'H', 'D', 'R'):
      {
        uint32_t idx = 0;
        littleEndianUint32 val;
        READ_INT_SWAP(val, buf_, (offset_ + idx));
        width_ = val.value, idx += 4;
        READ_INT_SWAP(val, buf_, (offset_ + idx));
        height_ = val.value, idx += 4;

        bpp_ = static_cast<uint32_t> (buf_[offset_ + idx++]);
        colorType_ = static_cast<uint32_t>(buf_[offset_ + idx++]);
        compressType_ = static_cast<uint32_t>(buf_[offset_ + idx++]);
        filterType_ = static_cast<uint32_t>(buf_[offset_ + idx++]);
        interlaceType_ = static_cast<uint32_t>(buf_[offset_ + idx++]);

        offset_ += sizeof(uint32_t) + len.value; // passing CRC 4 bytes too
        break;
      }
      case PNG_CHUNCK('g', 'A', 'M', 'A'):
      {
        littleEndianUint32 encodedGamma;
        READ_INT_SWAP(encodedGamma, buf_, offset_);
        gamma_ = encodedGamma.value / PNG_INTEGER_ENCODING_FACTOR;
        offset_ += sizeof(uint32_t) + len.value;
        break;
      }
      case PNG_CHUNCK('c', 'H', 'R', 'M'):
      {
        littleEndianUint32 val;
        ASSERT(len.value == (2 * 4 * 4), "cHRM Chunk length is not 16(%d)", len.value);
        for(int idx = 0; idx < 4; idx++) {
          READ_INT_SWAP(val, buf_, (offset_ + idx * 8));
          chrm_[idx].x = val.value / PNG_INTEGER_ENCODING_FACTOR;
          READ_INT_SWAP(val, buf_, (offset_ + idx * 8 + 4));
          chrm_[idx].y = val.value / PNG_INTEGER_ENCODING_FACTOR;
        }
        hasChrm_ = true;
        offset_ += sizeof(uint32_t) + len.value;
        break;
      }
      case PNG_CHUNCK('s', 'R', 'G', 'B'):
      {
        has_sRGB = true;
        ASSERT(len.value == 1, "sRGB length error");
        offset_ += sizeof(uint32_t) + len.value;
        break;
      }
      case PNG_CHUNCK('i', 'C', 'C', 'P'):
        LOGI("====iCCP: %s, compression Method %d",
             &buf_[offset_],
             buf_[offset_ +
                 strlen(reinterpret_cast<const char*>(&buf_[offset_])) + 1]);
        has_iCCP = true;
        offset_ += sizeof(uint32_t) + len.value;
        break;
      default:
        LOGV("====Unprocessed CHUNK %c%c%c%c",
             type.bytes[3], type.bytes[2], type.bytes[1], type.bytes[0]);
        [[fallthrough]];
      case PNG_CHUNCK('I', 'D', 'A', 'T'):
        [[fallthrough]];
      case PNG_CHUNCK('i', 'T', 'X','t'):
        [[fallthrough]];
      case PNG_CHUNCK('t', 'T', 'X', 't'):
        [[fallthrough]];
      case PNG_CHUNCK('z', 'T', 'X', 't'):
        [[fallthrough]];
      case PNG_CHUNCK('t', 'I', 'M', 'E'):
        [[fallthrough]];
      case PNG_CHUNCK('I', 'E', 'N', 'D'):
        offset_ += sizeof(uint32_t) + len.value;  // CRC + data_size
        break;
    }
  }

  if(has_sRGB) {
    CIE_POINT defaultsRGBValues[] = {
        {.31270f, .32900f },
        {.640000f, .33f   },
        {.3f,      .6f    },
        {.15f,     .06f   },
    };
    memcpy(chrm_, defaultsRGBValues, sizeof(defaultsRGBValues));
    gamma_ = .45455f;
    hasChrm_ = true;
  }

  if(hasChrm_) {
    UpdateNPM();
  }

  /*
   * shortcut: if SRGB / iCCP present, assume to be P3 iamge
   */
  if (has_sRGB || has_iCCP) {
    hasChrm_ = false;   // When no cHRM present, assume to be P3 colorspace
  }
  valid_ = true;
}

float PNGHeader::GetGamma() const{
    return gamma_;
}

/*
 * Is the picture coded in display_ P3 space?
 * Taking agressive path:
 *    1) if has cHRM chunk
 *    2) values are closer to sRGB than to P3 reference points
 * Otherwise, all assumed to be P3 space
 */
#define UNSIGNED_DELTA(x, y) ((x) > (y) ? (x) - (y) : (y) - (x))
bool  PNGHeader::IsP3Image(void) const {
  if (!hasChrm_) {
    return true;
  }
  CIE_POINT p3[] = {
      {.31270f, .32900f},   // white
      {.68000f, .32000f},   // red
      {.26500f, .69000f},   // green
      {.15000f, .06000f},   // blue
  };

  CIE_POINT srgb[] = {
      {.31270f, .3290f},
      {.64000f, .3300f},
      {.30000f, .6000f},
      {.15000f, .0600f},
  };
  //calculate distance to P3 reference points and sRGB ref points
  float   toP3 = 0.0f, toSRGB = 0.0f;
  for (int32_t idx = 1; idx < 4; idx++) {
    toP3 += (chrm_[idx].x - p3[idx].x) * (chrm_[idx].x - p3[idx].x);
    toP3 += (chrm_[idx].y - p3[idx].y) * (chrm_[idx].y - p3[idx].y);

    toSRGB += (chrm_[idx].x - srgb[idx].x) * (chrm_[idx].x - srgb[idx].x);
    toSRGB += (chrm_[idx].y - srgb[idx].y) * (chrm_[idx].y - srgb[idx].y);
  }

  if (toSRGB > toP3) {
    return true;
  }
  return false;
}

/*
 * Calculate NPM, refer to SMPTE RP-177-1993
 */
void PNGHeader::UpdateNPM(void) {

  if (!hasChrm_)
    return;

  mathfu::mat3 npm(chrm_[1].x, chrm_[1].y, 1.0f - chrm_[1].x - chrm_[1].y,
                   chrm_[2].x, chrm_[2].y, 1.0f - chrm_[2].x - chrm_[2].y,
                   chrm_[3].x, chrm_[3].y, 1.0f - chrm_[3].x - chrm_[3].y);
  mathfu::vec3 w(chrm_[0].x / chrm_[0].y, 1.0f,
                 (1.0f - chrm_[0].x - chrm_[0].y)/chrm_[0].y);

  mathfu::vec3 c = npm.Inverse() * w;
  mathfu::mat3 diagMat = mathfu::mat3::Identity();
  diagMat(0,0) = c[0], diagMat(1,1) = c[1], diagMat(2,2) = c[2];

  NPM_ = npm * diagMat;
}

bool PNGHeader::HasNPM(void) const {
  return hasChrm_;
}

const mathfu::mat3* PNGHeader::NPM(void) {
  ASSERT(hasChrm_, "File does not have NPM info");
  return &NPM_;
}
