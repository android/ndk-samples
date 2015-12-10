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

/* We're going to perform computations for every pixel of the target
 * bitmap. floating-point operations are very slow on ARMv5, and not
 * too bad on ARMv7 with the exception of trigonometric functions.
 *
 * For better performance on all platforms, we're going to use fixed-point
 * arithmetic and all kinds of tricks
 */

const int PlasmaRenderer::FIXED_BITS = 16;
const PlasmaRenderer::Fixed PlasmaRenderer::FIXED_ONE = 1 << FIXED_BITS;

// ANGLE_BITS must be at least 8
const int PlasmaRenderer::ANGLE_BITS = 9;

const PlasmaRenderer::Angle PlasmaRenderer::ANGLE_2PI = 1 << ANGLE_BITS;
const PlasmaRenderer::Angle PlasmaRenderer::ANGLE_PI  = 1 << (ANGLE_BITS-1);
const PlasmaRenderer::Angle PlasmaRenderer::ANGLE_PI2 = 1 << (ANGLE_BITS-2);

PlasmaRenderer::Angle PlasmaRenderer::angle_from_fixed(PlasmaRenderer::Fixed x) { return (PlasmaRenderer::Angle)(x >> (PlasmaRenderer::FIXED_BITS - PlasmaRenderer::ANGLE_BITS)); }

PlasmaRenderer::Fixed PlasmaRenderer::angle_sin_tab[ANGLE_2PI+1];

void PlasmaRenderer::init_angles() {
  for (int nn = 0; nn < ANGLE_2PI + 1; nn++) {
    double radians = nn * M_PI / ANGLE_PI;
    angle_sin_tab[nn] = fixed_from_float(sin(radians));
  }
}

PlasmaRenderer::Fixed PlasmaRenderer::angle_sin(Angle a) {
  return angle_sin_tab[(uint32_t) a & (ANGLE_2PI - 1)];
}

PlasmaRenderer::Fixed PlasmaRenderer::angle_cos(Angle a) {
  return angle_sin(a + ANGLE_PI2);
}

PlasmaRenderer::Fixed PlasmaRenderer::fixed_sin(Fixed f) {
  return angle_sin(angle_from_fixed(f));
}

PlasmaRenderer::Fixed  PlasmaRenderer::fixed_cos(Fixed f) {
  return angle_cos(angle_from_fixed(f));
}

PlasmaRenderer::Fixed PlasmaRenderer::fixed_from_float(float x) {
  return (PlasmaRenderer::Fixed)(x*FIXED_ONE);
}

int PlasmaRenderer::fixed_frac(PlasmaRenderer::Fixed x) {
  return x & ((1 << FIXED_BITS)-1);
}