/*
 * Copyright 2020 The Android Open Source Project
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

#ifndef TEAPOTS_IMAGEDECODERRENDER_H
#define TEAPOTS_IMAGEDECODERRENDER_H
#include "TeapotRenderer.h"
#include "Texture.h"
/**
 *  class TextureTeapotRender
 *    adding texture into teapot
 *     - create texture buffer and load image in assets/Textures
 *     - enable texture units
 *     - enable texturing inside shaders
 */
class ImageDecoderRender : public TeapotRenderer {
  GLuint texVbo_ = GL_INVALID_VALUE;
  Texture* texObj_ = nullptr;

 public:
  ImageDecoderRender();
  virtual ~ImageDecoderRender();
  // This is to decide which teapot type to render:
  //   plain teapot
  //   2D textured teapot
  //   Cubemap textured teapot
  // the rest of the code looks this function to decide
  // what to render.
  virtual GLint GetTextureType(void);
  virtual void Init(AAssetManager* amgr);
  virtual void Render();
  virtual void Unload();
};

#endif  // TEAPOTS_IMAGEDECODERRENDER_H
