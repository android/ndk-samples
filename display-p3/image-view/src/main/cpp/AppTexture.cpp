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
#include "AssetUtil.h"
#include "ImageViewEngine.h"

/*
 * DeleteTextures()
 *    Release all textures created in engine
 */
void ImageViewEngine::DeleteTextures(void) {
  for (auto& tex : textures_) {
    delete tex;
  }
  textures_.resize(0);
}

/*
 * CreateTextures()
 *    Create 2 textures in current display_ color space ( P3 or sRGB)
 *    If it is P3 space, image is transformed through sRGB so colors
 *    outside sRGB gamut are removed.
 */
bool ImageViewEngine::CreateTextures(void) {
  std::vector<std::string> files;
  AssetEnumerateFileType(app_->activity->assetManager, ".png", files);
  if (files.empty()) {
    return false;
  }
  DeleteTextures();

  for(auto& f : files) {
    AssetTexture* tex = new AssetTexture(f);
    ASSERT(tex, "OUT OF MEMORY");
    tex->ColorSpace(dispColorSpace_);
    bool status = tex->CreateGLTextures(app_->activity->assetManager);
    ASSERT(status, "Failed to create Texture for %s", f.c_str());
    textures_.push_back(tex);
  }

  return true;
}
