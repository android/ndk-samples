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

#ifndef  __SIMPLE_PNG_H__
#define  __SIMPLE_PNG_H__

#include <cstdint>
#include <string>
#include "android_debug.h"
#include <mathfu/glsl_mappings.h>

#pragma pack(push, 1 )

struct CIE_POINT {
  float x, y;
};

#define REF_WHITE_IDX 0
#define REF_RED_IDX   1
#define REF_GREEN_IDX 2
#define REF_BLUE_IDX  3

#define PNG_INTEGER_ENCODING_FACTOR 100000.0f
/*
 * Default Gamma if image file does not have Gamma
 */
#define DEFAULT_IMAGE_GAMMA  (1.0f/2.2f)

class PNGHeader {
public:
  explicit PNGHeader(std::string& name, uint8_t* buf, uint64_t len);
  ~PNGHeader() {
    buf_ = nullptr, offset_= 0;
  }

  float GetGamma(void) const;
  bool  IsP3Image(void) const;
  bool  HasNPM(void) const;
  const mathfu::mat3* NPM(void);

private:
  void UpdateNPM(void);

  std::string name_;
  uint8_t* buf_;
  uint64_t length_;
  uint64_t offset_;
  float  gamma_;

  // header info:
  uint32_t width_, height_, bpp_, colorType_;
  uint32_t compressType_, filterType_, interlaceType_;
  CIE_POINT  chrm_[4];
  bool hasChrm_;
  mathfu::mat3 NPM_;
  bool  valid_;
};

#pragma pack(pop)

#endif //  __SIMPLE_PNG_H__


