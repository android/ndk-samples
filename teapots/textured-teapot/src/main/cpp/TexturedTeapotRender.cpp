/*
 * Copyright 2018 The Android Open Source Project
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

#include "TexturedTeapotRender.h"

/**
 * Texture Coordinators for 2D texture:
 *    they are declared in file model file teapot.inl with tiles
 *    for front and back teapot faces. If you do not want see
 *    the tiles but would like to see the stretched version, simply
 *    divide the texCoord by 2. Macro TILED_TEXTURE is for this purpose.
 *
 * teapot.inl file already included in TeapotRenderer.cpp, we directly
 * use! -- nice
 */
#define TILED_TEXTURE 0

extern float teapotTexCoords[];

/**
 * Constructor: all work is done inside Init() function.
 *              nothing to do here
 */
TexturedTeapotRender::TexturedTeapotRender() {}

/**
 * Destructor:
 *     let Unload() do the work, which should also trigger
 *     TeapotRenderer's Unload() function
 */
TexturedTeapotRender::~TexturedTeapotRender() { Unload(); };

/**
 * Init: Initialize the GL with needed data. We add on the things
 * needed for textures
 *  - load image data into generated glBuffers
 *  - configure samplerObj in fragment shader
 * @param assetMgr android assetManager from java side
 */
void TexturedTeapotRender::Init(AAssetManager* assetMgr) {
  // initialize the basic things from TeapotRenderer, no change
  TeapotRenderer::Init();

  // Need flip Y, so as top/bottom image
  std::vector<std::string> textures{
      std::string("Textures/right.tga"),   // GL_TEXTURE_CUBE_MAP_POSITIVE_X
      std::string("Textures/left.tga"),    // GL_TEXTURE_CUBE_MAP_NEGATIVE_X
      std::string("Textures/bottom.tga"),  // GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
      std::string("Textures/top.tga"),     // GL_TEXTURE_CUBE_MAP_POSITIVE_Y
      std::string("Textures/front.tga"),   // GL_TEXTURE_CUBE_MAP_POSITIVE_Z
      std::string("Textures/back.tga")     // GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
  };

  texObj_ = new Texture(textures, assetMgr);
  assert(texObj_);

  std::vector<std::string> samplers;
  std::vector<GLint> units;
  texObj_->GetActiveSamplerInfo(samplers, units);
  for (size_t idx = 0; idx < samplers.size(); idx++) {
    GLint sampler =
        glGetUniformLocation(shader_param_.program_, samplers[idx].c_str());
    glUniform1i(sampler, units[idx]);
  }

  texObj_->Activate();
}

/**
 * Render() function:
 *   enable states for rendering and reader a frame.
 *   For Texture, simply inform GL to stream texture coord from _texVbo
 */
void TexturedTeapotRender::Render() { TeapotRenderer::Render(); }

/**
 * Unload()
 *    clean-up function. May get called from destructor too
 */
void TexturedTeapotRender::Unload() {
  TeapotRenderer::Unload();
  if (texVbo_ != GL_INVALID_VALUE) {
    glDeleteBuffers(1, &texVbo_);
    texVbo_ = GL_INVALID_VALUE;
  }
  if (texObj_) {
    delete texObj_;
    texObj_ = nullptr;
  }
}
