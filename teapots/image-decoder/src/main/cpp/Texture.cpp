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

#include <GLES3/gl32.h>
#include <android/data_space.h>
#include <assert.h>

#include "Texture.h"
#include <android/imagedecoder.h>

#define MODULE_NAME "Teapot::Texture"
#include "android_debug.h"

/**
 * Load2DTextureFromAsset(): Load one RGBA 2d texture from asset into
 * the given GL texture with NDK's ImageDecoder interface.
 */
static bool Load2DTextureFromAsset(std::string& assetFile, AAssetManager* mgr,
                                   GLenum target) {
  int32_t imgWidth, imgHeight;
  std::vector<uint8_t> imgBits;

  // Open the asset with the give name from the APK's assets folder.
  AAsset* assetDescriptor =
      AAssetManager_open(mgr, assetFile.c_str(), AASSET_MODE_BUFFER);
  ASSERT(assetDescriptor, "%s does not exist in %s", assetFile.c_str(),
         __FUNCTION__);

  // Create an AImageDecoder from the given AAsset
  AImageDecoder* decoder = nullptr;
  int status = AImageDecoder_createFromAAsset(assetDescriptor, &decoder);
  ASSERT(ANDROID_IMAGE_DECODER_SUCCESS == status,
         "Failed to create ImageDecoder for %s", assetFile.c_str());

  status = AImageDecoder_setAndroidBitmapFormat(
      decoder, ANDROID_BITMAP_FORMAT_RGBA_8888);
  ASSERT(ANDROID_IMAGE_DECODER_SUCCESS == status,
         "Failed to request 8888 output format for %s", assetFile.c_str());

  status = AImageDecoder_setUnpremultipliedRequired(decoder, true);
  ASSERT(ANDROID_IMAGE_DECODER_SUCCESS == status,
         "Failed to bypass pre-multiply alpha for %s", assetFile.c_str());

  const AImageDecoderHeaderInfo* headerInfo =
      AImageDecoder_getHeaderInfo(decoder);
  ASSERT(headerInfo != nullptr, "Failed to get ImageHeaderInfo for %s",
         assetFile.c_str());

  // scRGB/sRGB/SCRGB_LINEAR are okay for this sample
  ADataSpace dataSpace =
      static_cast<ADataSpace>(AImageDecoderHeaderInfo_getDataSpace(headerInfo));
  if (dataSpace != ADATASPACE_SCRGB && dataSpace != ADATASPACE_SRGB &&
      dataSpace != ADATASPACE_SCRGB_LINEAR) {
    status = AImageDecoder_setDataSpace(decoder, ADATASPACE_SRGB);
    ASSERT(ANDROID_IMAGE_DECODER_SUCCESS == status,
           "Failed to set SRGB color space %s", assetFile.c_str());
  }

  imgWidth = AImageDecoderHeaderInfo_getWidth(headerInfo);
  imgHeight = AImageDecoderHeaderInfo_getHeight(headerInfo);
  size_t stride = AImageDecoder_getMinimumStride(decoder);

  // by design, ImageDecoder decode the image into packed format, no padding.
  // Let's make sure there is no padding; otherwise it would be bug, and app
  // need to pack.
  ASSERT(stride == imgWidth * 4, "ImageDecoder padded  decoded image");

  imgBits.clear();
  imgBits.resize(imgHeight * stride);

  status = AImageDecoder_decodeImage(decoder, imgBits.data(), stride,
                                     imgBits.size());
  ASSERT(status == ANDROID_IMAGE_DECODER_SUCCESS, "Failed to decode image %s",
         assetFile.c_str());

  glTexImage2D(target, 0, GL_RGBA, imgWidth, imgHeight, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, imgBits.data());

  // release decoder and asset
  AImageDecoder_delete(decoder);
  AAsset_close(assetDescriptor);
  return true;
}

/**
 * Cubemap and Texture2d implementations for Class Texture.
 */
class TextureCubemap : public Texture {
 protected:
  GLuint texId_ = GL_INVALID_VALUE;
  bool activated_ = false;

 public:
  virtual ~TextureCubemap();
  TextureCubemap(std::vector<std::string>& texFiles,
                 AAssetManager* assetManager);
  virtual bool GetActiveSamplerInfo(std::vector<std::string>& names,
                                    std::vector<GLint>& units);
  virtual bool Activate(void);
  virtual GLuint GetTexType();
  virtual GLuint GetTexId();
};

class Texture2d : public Texture {
 protected:
  GLuint texId_ = GL_INVALID_VALUE;
  bool activated_ = false;

 public:
  virtual ~Texture2d();
  // Implement just one texture
  Texture2d(std::string& texFiles, AAssetManager* assetManager);
  virtual bool GetActiveSamplerInfo(std::vector<std::string>& names,
                                    std::vector<GLint>& units);
  virtual bool Activate(void);
  virtual GLuint GetTexType();
  virtual GLuint GetTexId();
};

/**
 * Capability debug string
 */
static const std::string supportedTextureTypes =
    "GL_TEXTURE_2D(0x0DE1) GL_TEXTURE_CUBE_MAP(0x8513)";

/**
 * Interface implementations
 */
Texture::Texture() {}
Texture::~Texture() {}
/**
 * Create Texture Object
 * @param texFiles holds the texture file name(s) under APK's assets
 * @param type should be one (GL_TEXTURE_2D / GL_TEXTURE_CUBE_MAP)
 * @param assetManager is used to open texture files inside assets
 * @return is the newly created Texture Object
 */
Texture* Texture::Create(GLuint type, std::vector<std::string>& texFiles,
                         AAssetManager* assetManager) {
  if (type == GL_TEXTURE_2D) {
    return dynamic_cast<Texture*>(new Texture2d(texFiles[0], assetManager));
  } else if (type == GL_TEXTURE_CUBE_MAP) {
    return dynamic_cast<Texture*>(new TextureCubemap(texFiles, assetManager));
  }

  LOGE("Unknown texture type %x to created", type);
  LOGE("Supported Texture Types: %s", supportedTextureTypes.c_str());
  assert(false);
  return nullptr;
}

void Texture::Delete(Texture* obj) {
  if (obj == nullptr) {
    ASSERT(false, "NULL pointer to Texture::Delete() function");
    return;
  }

  GLuint type = obj->GetTexType();
  if (type == GL_TEXTURE_2D) {
    Texture2d* d2Instance = dynamic_cast<Texture2d*>(obj);
    if (d2Instance) {
      delete d2Instance;
    } else {
      ASSERT(false, "Unknown obj type to %s", __FUNCTION__);
    }
  } else if (type == GL_TEXTURE_CUBE_MAP) {
    TextureCubemap* cubemapInstance = dynamic_cast<TextureCubemap*>(obj);
    if (cubemapInstance) {
      delete cubemapInstance;
    } else {
      ASSERT(false, "Unknown obj type to %s", __FUNCTION__);
    }
  } else {
    LOGE("Supported Texture Types: %s", supportedTextureTypes.c_str());
    ASSERT(false, "Unknow texture type %x to delete", type);
  }
}

/**
 * TextureCubemap implementations
 */
bool TextureCubemap::Activate(void) {
  assert(texId_ != GL_INVALID_VALUE);

  glBindTexture(texId_, GL_TEXTURE0);
  glActiveTexture(GL_TEXTURE0 + 0);
  activated_ = true;
  return true;
}

GLuint TextureCubemap::GetTexType() { return GL_TEXTURE_CUBE_MAP; }

GLuint TextureCubemap::GetTexId() {
  assert(texId_ != GL_INVALID_VALUE);
  return texId_;
}

TextureCubemap::TextureCubemap(std::vector<std::string>& files,
                               AAssetManager* mgr) {
  // For Cubemap, we use world normal to sample the textures
  // so no texture vbo necessary

  if (!mgr || files.size() != 6) {
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
    Load2DTextureFromAsset(files[i], mgr, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i);
  }

  glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_REPEAT);

  glActiveTexture(GL_TEXTURE0);
  activated_ = true;
}

/**
 * Dtor
 *    clean-up function
 */
TextureCubemap::~TextureCubemap() {
  if (texId_ != GL_INVALID_VALUE) {
    glDeleteTextures(1, &texId_);
    texId_ = GL_INVALID_VALUE;
    activated_ = false;
  }
}

/**
  Return used sampler names and units
      so application could configure shader's sampler uniform(s).
  Cubemap just used one sampler at unit 0 with "samplerObj" as its name.
 */
bool TextureCubemap::GetActiveSamplerInfo(std::vector<std::string>& names,
                                          std::vector<GLint>& units) {
  names.clear();
  names.push_back(std::string("samplerObj"));
  units.clear();
  units.push_back(0);

  return true;
}

/**
 * Texture2D implementation
 */
Texture2d::Texture2d(std::string& fileName, AAssetManager* assetManager) {
  if (!assetManager) {
    LOGE("AssetManager to Texture2D() could not be null!!!");
    assert(false);
    return;
  }

  glGenTextures(1, &texId_);
  glBindTexture(GL_TEXTURE_2D, texId_);

  if (texId_ == GL_INVALID_VALUE) {
    assert(false);
    return;
  }

  Load2DTextureFromAsset(fileName, assetManager, GL_TEXTURE_2D);

  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glActiveTexture(GL_TEXTURE0);
}

Texture2d::~Texture2d() {
  if (texId_ != GL_INVALID_VALUE) {
    glDeleteTextures(1, &texId_);
    texId_ = GL_INVALID_VALUE;
  }
  activated_ = false;
}

/**
 * Same as the Cubemap::GetActiveSamplerInfo()
 */
bool Texture2d::GetActiveSamplerInfo(std::vector<std::string>& names,
                                     std::vector<GLint>& units) {
  names.clear();
  names.push_back(std::string("samplerObj"));
  units.clear();
  units.push_back(0);

  return true;
}

bool Texture2d::Activate(void) {
  glBindTexture(texId_, GL_TEXTURE0);
  glActiveTexture(GL_TEXTURE0 + 0);
  activated_ = true;
  return true;
}

GLuint Texture2d::GetTexType() { return GL_TEXTURE_2D; }

GLuint Texture2d::GetTexId() { return texId_; }
