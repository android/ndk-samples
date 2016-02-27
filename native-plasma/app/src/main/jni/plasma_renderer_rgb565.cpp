/*
 * Copyright (C) 2015 The Android Open Source Project
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
#include "plasma_renderer.h"

// Set to 1 to optimize memory stores when generating plasma.
#define OPTIMIZE_WRITES 1

/* Color palette used for rendering the plasma */
static const int PALETTE_BITS = 8;
static const int PALETTE_SIZE = 1 << PALETTE_BITS;

static uint16_t palette[PALETTE_SIZE];

static uint16_t makeColor(int red, int green, int blue) {
  return (uint16_t) (((red << 8) & 0xf800) |
                     ((green << 2) & 0x03e0) |
                     ((blue >> 3) & 0x001f));
}

void PlasmaRendererRGB565::init_palette() {
  int nn, mm = 0;
  // fun with colors
  for (nn = 0; nn < PALETTE_SIZE / 4; nn++) {
    int jj = (nn - mm) * 4 * 255 / PALETTE_SIZE;
    palette[nn] = makeColor(255, jj, 255 - jj);
  }

  for (mm = nn; nn < PALETTE_SIZE / 2; nn++) {
    int jj = (nn - mm) * 4 * 255 / PALETTE_SIZE;
    palette[nn] = makeColor(255 - jj, 255, jj);
  }

  for (mm = nn; nn < PALETTE_SIZE * 3 / 4; nn++) {
    int jj = (nn - mm) * 4 * 255 / PALETTE_SIZE;
    palette[nn] = makeColor(0, 255 - jj, 255);
  }

  for (mm = nn; nn < PALETTE_SIZE; nn++) {
    int jj = (nn - mm) * 4 * 255 / PALETTE_SIZE;
    palette[nn] = makeColor(jj, 0, 255);
  }
}

uint16_t PlasmaRendererRGB565::palette_from_fixed(Fixed x) {
  if (x < 0) x = -x;
  if (x >= FIXED_ONE) x = FIXED_ONE - 1;
  int idx = fixed_frac(x) >> (FIXED_BITS - PALETTE_BITS);
  return palette[idx & (PALETTE_SIZE - 1)];
}

void PlasmaRendererRGB565::init() {
  init_palette();
  init_angles();
}

void PlasmaRendererRGB565::fill_plasma(ANativeWindow_Buffer *buffer, double t) {
  Fixed yt1 = fixed_from_float(t / 1230.);
  Fixed yt2 = yt1;
  Fixed xt10 = fixed_from_float(t / 3000.);
  Fixed xt20 = xt10;

  static const Fixed YT1_INCR = fixed_from_float(1/100.);
  static const Fixed YT2_INCR = fixed_from_float(1/163.);
  static const Fixed XT1_INCR = fixed_from_float(1/173.);
  static const Fixed XT2_INCR = fixed_from_float(1/242.);

  void *pixels = buffer->bits;
  //LOGI("width=%d height=%d stride=%d format=%d", buffer->width, buffer->height,
  //        buffer->stride, buffer->format);

  for (int yy = 0; yy < buffer->height; yy++) {
    uint16_t *line = (uint16_t *) pixels;
    Fixed base = fixed_sin(yt1) + fixed_sin(yt2);
    Fixed xt1 = xt10;
    Fixed xt2 = xt20;

    yt1 += YT1_INCR;
    yt2 += YT2_INCR;

#if OPTIMIZE_WRITES
    // optimize memory writes by generating one aligned 32-bit store
    // for every pair of pixels.
    uint16_t *line_end = line + buffer->width;

    if (line < line_end) {
      if (((uint32_t) (uintptr_t) line & 3) != 0) {
        Fixed ii = base + fixed_sin(xt1) + fixed_sin(xt2);

        xt1 += XT1_INCR;
        xt2 += XT2_INCR;

        line[0] = palette_from_fixed(ii >> 2);
        line++;
      }

      while (line + 2 <= line_end) {
        Fixed i1 = base + fixed_sin(xt1) + fixed_sin(xt2);
        xt1 += XT1_INCR;
        xt2 += XT2_INCR;

        Fixed i2 = base + fixed_sin(xt1) + fixed_sin(xt2);
        xt1 += XT1_INCR;
        xt2 += XT2_INCR;

        uint32_t pixel = ((uint32_t) palette_from_fixed(i1 >> 2) << 16) |
                         (uint32_t) palette_from_fixed(i2 >> 2);

        ((uint32_t *) line)[0] = pixel;
        line += 2;
      }

      if (line < line_end) {
        Fixed ii = base + fixed_sin(xt1) + fixed_sin(xt2);
        line[0] = palette_from_fixed(ii >> 2);
        line++;
      }
    }
#else // !OPTIMIZE_WRITES
    for ( int xx = 0; xx < buffer->width; xx++ ) {

          Fixed ii = base + fixed_sin(xt1) + fixed_sin(xt2);

          xt1 += XT1_INCR;
          xt2 += XT2_INCR;

          line[xx] = palette_from_fixed(ii / 4);
      }
#endif // !OPTIMIZE_WRITES

    // go to next line
    pixels = (uint16_t *) pixels + buffer->stride;
  }
}