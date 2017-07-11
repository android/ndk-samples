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

#ifndef  __COLOR_TRANSFORM_H__
#define __COLOR_TRANSFORM_H__

#include <cstdint>
#include <mathfu/glsl_mappings.h>

struct IMAGE_FORMAT {
  void*       buf_;  // packed image pointer
  uint32_t    width_, height_;
  float       gamma_;
  const mathfu::mat3* npm_;
};

#define DEFAULT_DISPLAY_GAMMA (1.0f/2.2f)
#define DEFAULT_P3_IMAGE_GAMMA (1.0f/2.2f)

/*
 * TransformColorSpace(IMAGE_FORMAT& dst, IMAGE_FORMAT& src)
 *     Transforms image between DCI-P3 and sRGB space
 *     Dst.buf_ = dst.npm * src.npm * de-gamma(src.buf_)
 *     dst.buf_ = en-gamma(dst.buf_)
 * dst.buf_:
 *     transformed image buf pointer; user must allocate enough space for the image
 * src.buf_:
 *     source of the image bits to transform.
 * Both src and dst must be in:
 *     R8G8B8A8 4 channels packed format
 */
bool TransformColorSpace(IMAGE_FORMAT &dst, IMAGE_FORMAT& src);

/*
 * GetTransformNPM
 */
enum NPM_TYPE {
  SRGB_D65 = 0,
  SRGB_D65_INV,
  P3_D65,
  P3_D65_INV,
  TYPE_COUNT
};
const mathfu::mat3* GetTransformNPM(NPM_TYPE type);

#endif // __COLOR_TRANSFORM_H__
