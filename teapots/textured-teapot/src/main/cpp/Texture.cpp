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

#include "Texture.h"

#include <GLES3/gl32.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include "AssetUtil.h"

#define MODULE_NAME "Teapot::Texture"
#include "android_debug.h"

/**
 * Interface implementations
 */
Texture::Texture(std::vector<std::string>& asset_paths,
                 AAssetManager* asset_manager) {
  // For Cubemap, we use world normal to sample the textures
  // so no texture vbo necessary

  int32_t imgWidth, imgHeight, channelCount;
  std::vector<uint8_t> fileBits;

  if (!asset_manager || asset_paths.size() != 6) {
    assert(false);
    return;
  }

  glGenTextures(1, &texId_);
  glBindTexture(GL_TEXTURE_CUBE_MAP, texId_);

  if (texId_ == GL_INVALID_VALUE) {
    assert(false);
    return;
  }

  for (GLuint i = 0; i < 6; i++) {
    fileBits.clear();
    AssetReadFile(asset_manager, asset_paths[i], fileBits);

    // tga/bmp asset_paths are saved as vertical mirror images ( at least more than
    // half ).
    stbi_set_flip_vertically_on_load(1);

    uint8_t* imageBits =
        stbi_load_from_memory(fileBits.data(), fileBits.size(), &imgWidth,
                              &imgHeight, &channelCount, 4);

    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, imgWidth,
                 imgHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageBits);
    stbi_image_free(imageBits);
  }

  glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_REPEAT);

  glActiveTexture(GL_TEXTURE0);
}

Texture::~Texture() {
  if (texId_ != GL_INVALID_VALUE) {
    glDeleteTextures(1, &texId_);
    texId_ = GL_INVALID_VALUE;
  }
}

/**
 * TextureCubemap implementations
 */
bool Texture::Activate(void) {
  assert(texId_ != GL_INVALID_VALUE);

  glBindTexture(texId_, GL_TEXTURE0);
  glActiveTexture(GL_TEXTURE0 + 0);
  return true;
}

/**
  Return used sampler names and units
      so application could configure shader's sampler uniform(s).
  Cubemap just used one sampler at unit 0 with "samplerObj" as its name.
 */
void Texture::GetActiveSamplerInfo(std::vector<std::string>& names,
                                          std::vector<GLint>& units) {
  names.clear();
  names.push_back(std::string("samplerObj"));
  units.clear();
  units.push_back(0);
}
