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
#ifndef PLASMA_RENDERER_H
#define PLASMA_RENDERER_H

#include <android_native_app_glue.h>
#include <math.h>
#include <stdint.h>

// Virtual class implemented by each type of plasma renderer for each framebuffer format
class PlasmaRenderer {
public:
  typedef int32_t Fixed;
  // Angles are expressed as fixed point radians
  typedef int32_t Angle;

  virtual void init() = 0;

  virtual void fill_plasma(ANativeWindow_Buffer *buffer, double t) = 0;

protected:
  void init_angles();

  Fixed angle_sin(Angle a);

  Fixed angle_cos(Angle a);

  Fixed fixed_sin(Fixed f);

  Fixed fixed_cos(Fixed f);

  Fixed fixed_from_float(float x);

  int fixed_frac(Fixed x);

  Angle angle_from_fixed(Fixed x);
  static const int FIXED_BITS;
  static const Fixed FIXED_ONE;
  static const int ANGLE_BITS;
  static const Angle ANGLE_2PI;
  static const Angle ANGLE_PI;
  static const Angle ANGLE_PI2;
  static Fixed angle_sin_tab[];
};

class PlasmaRendererRGBA8888 : public PlasmaRenderer {
public:
  void init();

  void fill_plasma(ANativeWindow_Buffer *buffer, double t);

private:
  void init_palette();

  inline uint32_t palette_from_fixed(Fixed x);
};

class PlasmaRendererRGB565 : public PlasmaRenderer {
public:
  void init();

  void fill_plasma(ANativeWindow_Buffer *buffer, double t);

private:
  void init_palette();

  inline uint16_t palette_from_fixed(Fixed x);
};

#endif // PLASMA_RENDERER_H