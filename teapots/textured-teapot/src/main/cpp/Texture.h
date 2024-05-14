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

#pragma once

#include <EGL/egl.h>
#include <GLES/gl.h>
#include <android/asset_manager.h>

#include <string>
#include <vector>

/**
 *  class Texture
 *    adding texture into teapot
 *     - oad image in assets/Textures
 *     - enable texture units
 *     - report samplers needed inside shader
 *  Functionality wise:
 *     - one texture
 *     - one sampler
 *     - texture unit 0, sampler unit 0
 */
class Texture {
 public:
  /**
   * Create a texture object
   *
   * @param asset_paths holds image file names under APK/assets.
   *     cube map needs 6 (direction of +x, -x, +y, -y, +z, -z)
   * @param asset_manager Java side asset_manager object
   * @return newly created texture object, or nullptr in case of errors
   */
  Texture(std::vector<std::string>& asset_paths, AAssetManager* asset_manager);
  Texture(const Texture&) = delete;
  ~Texture();

  Texture& operator=(const Texture&) = delete;

  void GetActiveSamplerInfo(std::vector<std::string>& names,
                            std::vector<GLint>& units);
  bool Activate(void);

 private:
  GLuint texId_;
};
