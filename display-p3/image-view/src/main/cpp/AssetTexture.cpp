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

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include <stb/stb_image.h>
#include "simple_png.h"
#include "ColorSpaceTransform.h"
#include "AssetTexture.h"
#include "AssetUtil.h"
#include "ImageViewEngine.h"


#define INVALID_TEXTURE_ID 0xFFFFFFFF
AssetTexture::AssetTexture(const std::string& name) :
  name_(name), p3Id_(INVALID_TEXTURE_ID), sRGBId_(INVALID_TEXTURE_ID),
  valid_(false), dispColorSpace_(DISPLAY_COLORSPACE::INVALID)
{
}

AssetTexture::~AssetTexture() {
  if (valid_) {
    glDeleteTextures(1, &p3Id_);
    glDeleteTextures(1, &sRGBId_);
    valid_ = false;
    p3Id_ = INVALID_TEXTURE_ID;
    sRGBId_ = INVALID_TEXTURE_ID;
  }
}

void AssetTexture::ColorSpace(enum DISPLAY_COLORSPACE space) {
  ASSERT(space != DISPLAY_COLORSPACE::INVALID, "invalid dispColorSpace_");
  // should release all of the previous valid textures...
  dispColorSpace_ = space;
}
DISPLAY_COLORSPACE AssetTexture::ColorSpace(void) {
  return dispColorSpace_;
}

bool AssetTexture::IsValid(void) {
  return valid_;
}

GLuint AssetTexture::P3TexId() {
  ASSERT(valid_, "Texture has not created");
  return p3Id_;
}

GLuint AssetTexture::SRGBATexId() {
  ASSERT(valid_, "texture has not been created");
  return sRGBId_;
}

/*
 * CreateGLTexture()
 *     Create textures with regard to current display_ color space.
 *     For P3 image, one texture is created with original image; the second
 *     texture is created from:
 *       original image --> sRGB color Space --> display_ color space
 *     during the process, colors outside sRGB are clamped.
 */
bool AssetTexture::CreateGLTextures(AAssetManager *mgr) {
  ASSERT(mgr, "Asset Manager is not valid");
  ASSERT(dispColorSpace_ != DISPLAY_COLORSPACE::INVALID, "eglContext_ color space not set");
  if (valid_) {
    glDeleteTextures(1, &p3Id_);
    glDeleteTextures(1, &sRGBId_);
    valid_ = false;
    p3Id_ = INVALID_TEXTURE_ID;
    sRGBId_ = INVALID_TEXTURE_ID;
  }

  glGenTextures(1, &p3Id_);
  glBindTexture(GL_TEXTURE_2D, p3Id_);

  std::vector<uint8_t> fileData;
  AssetReadFile(mgr, name_, fileData);

  uint32_t imgWidth, imgHeight, n;
  uint8_t* imageData = stbi_load_from_memory(
      fileData.data(), fileData.size(), reinterpret_cast<int*>(&imgWidth),
      reinterpret_cast<int*>(&imgHeight), reinterpret_cast<int*>(&n), 4);
  uint8_t* imgBits = imageData;
  std::vector<uint8_t> staging;
  if (dispColorSpace_ == DISPLAY_COLORSPACE::SRGB) {
    staging.resize(imgWidth * imgHeight * 4 * sizeof(uint8_t));
    IMAGE_FORMAT src {
        .buf_ = imageData,
        .width_ = imgWidth,
        .height_ = imgHeight,
        .gamma_ = DEFAULT_P3_IMAGE_GAMMA,
        .npm_ = GetTransformNPM(NPM_TYPE::P3_D65),
    };

    IMAGE_FORMAT dst {
        .buf_ = staging.data(),
        .width_ = imgWidth,
        .height_ = imgHeight,
        .gamma_ = DEFAULT_DISPLAY_GAMMA,
        .npm_ = GetTransformNPM(NPM_TYPE::SRGB_D65_INV),
    };
    TransformColorSpace(dst, src);
    imgBits = staging.data();
  }

  // Our texture content is EOTF encoded, but depends on display P3 mode, app chooses to
  // use or bypass EOTF & OETF hardware functionality. See detailed comments in WideColorCtx.cpp
  // If OETF/EOTF needs bypassed on Android P and before, set flag for the texture to be in RGBA
  // to fake GPU to bypass OETF/EOTF ( gamma alike thing ).
  GLint textureInternalFormat = GL_SRGB8_ALPHA8;
  if (dispColorSpace_ == DISPLAY_COLORSPACE::P3_PASSTHROUGH) {
      textureInternalFormat = GL_RGBA;
  }
  glTexImage2D(GL_TEXTURE_2D, 0,  // mip level
               textureInternalFormat,
               imgWidth, imgHeight,
               0,                // border color
               GL_RGBA, GL_UNSIGNED_BYTE, imgBits);

  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

  // Generate sRGB view texture
  glGenTextures(1, &sRGBId_);
  glBindTexture(GL_TEXTURE_2D, sRGBId_);
  if(dispColorSpace_ == DISPLAY_COLORSPACE::P3 || dispColorSpace_ == DISPLAY_COLORSPACE::P3_PASSTHROUGH) {
    IMAGE_FORMAT src {
        .buf_ = imageData,
        .width_ = imgWidth,
        .height_ = imgHeight,
        .gamma_ = DEFAULT_P3_IMAGE_GAMMA,
        .npm_ = GetTransformNPM(NPM_TYPE::P3_D65),
    };
    std::vector<uint8_t> srgbImg(imgWidth * imgHeight * 4 * sizeof(uint8_t));
    IMAGE_FORMAT dst{
        .buf_ = srgbImg.data(),
        .width_ = imgWidth,
        .height_ = imgHeight,
        .gamma_ = 0.0f,     // intermediate image stays in linear space
        .npm_ = GetTransformNPM(NPM_TYPE::SRGB_D65_INV),
    };
    TransformColorSpace(dst, src);

    // sRGB back to P3 so we could display_ it correctly on P3 device mode
    staging.resize(imgWidth * imgHeight * 4 * sizeof(uint8_t));
    IMAGE_FORMAT tmp  = src;
    src = dst;   // intermediate gamma is 0.0f
    dst = tmp;   // original src's gamma is preserved
    src.npm_ = GetTransformNPM(NPM_TYPE::SRGB_D65);
    dst.buf_ = staging.data();
    dst.npm_ = GetTransformNPM(NPM_TYPE::P3_D65_INV);
    dst.gamma_ = DEFAULT_DISPLAY_GAMMA,

    TransformColorSpace(dst, src);
    imgBits = staging.data();
  }
  glTexImage2D(GL_TEXTURE_2D, 0,  // mip level
               textureInternalFormat, // GL_SRGB8_ALPHA8 for p3_ext mode,
                                      // GL_RGBA for p3_passthrough_ext
               imgWidth, imgHeight,
               0,                // border color
               GL_RGBA, GL_UNSIGNED_BYTE, imgBits);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

  glBindTexture(GL_TEXTURE_2D, 0);
  stbi_image_free(imageData);
  valid_ = true;

  return true;
}

std::string& AssetTexture::Name(void) {
  return name_;
}
