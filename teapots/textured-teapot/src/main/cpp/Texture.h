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

#ifndef TEAPOTS_TEXTURE_H
#define TEAPOTS_TEXTURE_H

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
  protected:
    Texture();
    virtual ~Texture();

  public:
    /**
     *   Create a texture object
     * @param type  should be GL_TEXTURE_2D / GL_TEXTURE_CUBE_MAP
     * @param texFiles holds image file names under APK/assets.
     *     2d texture uses the very first image texFiles[0]
     *     cube map needs 6 (direction of +x, -x, +y, -y, +z, -z)
     * @param assetManager Java side assetManager object
     * @return newly created texture object, or nullptr in case of errors
     */
    static Texture* Create( GLuint type, std::vector<std::string>& texFiles,
              AAssetManager* assetManager);
    static void Delete(Texture *obj);

    virtual bool GetActiveSamplerInfo(std::vector<std::string> &names,
                                      std::vector<GLint> &units) = 0;
    virtual bool Activate(void) = 0;
    virtual GLuint GetTexType() = 0;
    virtual GLuint GetTexId() = 0;

};
#endif //TEAPOTS_TEXTURE_H
