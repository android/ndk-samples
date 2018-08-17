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
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include "AssetUtil.h"
/**
 * Texture Coordinators:
 *    they are declared in file model file teapot.inl with tiles
 *    for front and back teapot faces. If you do not want see
 *    the tiles but would like to see the stretched version, simply
 *    divide the texCoord by 2. Macro TILED_TEXTURE is for this purpose.
 *
 * teapot.inl file already included in TeapotRenderer.cpp, we directly
 * use! -- nice
 */
#define TILED_TEXTURE 0

extern float  teapotTexCoords[];
constexpr  int32_t kCoordElementCount = (TILED_TEXTURE ? 3 : 2);

/**
 * LoadTexture()
 *   - Load given asset file ( bmp or tga types ) to OpenGL
 *   - Configure Texture filters
 *   - Activate Texture Stage 0
 */
bool TexturedTeapotRender::LoadTexture(const char* fileName, AAssetManager* mgr) {

    int32_t imgWidth, imgHeight, channelCount;
    std::string texName(fileName);
    std::vector<uint8_t> fileBits;

    if (!mgr) {
        assert(false);
        return false;
    }

    glGenTextures(1, &texId_);
    glBindTexture(GL_TEXTURE_2D, texId_);

    if(texId_ == GL_INVALID_VALUE) {
        assert(false);
        return false;
    }

    AssetReadFile(mgr, texName, fileBits);

    // tga/bmp files are saved as vertical mirror images ( at least more than half ).
    stbi_set_flip_vertically_on_load(1);

    uint8_t* imageBits = stbi_load_from_memory(
            fileBits.data(), fileBits.size(),
            &imgWidth, &imgHeight, &channelCount, 4);
    glTexImage2D(GL_TEXTURE_2D, 0,  // mip level
                 GL_RGBA,
                 imgWidth, imgHeight,
                 0,                // border color
                 GL_RGBA, GL_UNSIGNED_BYTE, imageBits);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

    glActiveTexture(GL_TEXTURE0);

    stbi_image_free(imageBits);

    return true;
}

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
TexturedTeapotRender::~TexturedTeapotRender() {
    Unload();
};

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

    // do Texture related initializations...
    glGenBuffers(1, &texVbo_);
    assert(texVbo_ != GL_INVALID_VALUE);

    /*
     * Loading Texture coord directly from data declared in model file
     *   teapot.inl
     * which is 3 floats/vertex.
     */
    glBindBuffer(GL_ARRAY_BUFFER, texVbo_);

#if (TILED_TEXTURE)
    glBufferData(GL_ARRAY_BUFFER,
                 kCoordElementCount * sizeof(float) * num_vertices_,
                 teapotTexCoords, GL_STATIC_DRAW);
#else
    std::vector<float> coords;
    for (int32_t idx = 0; idx < num_vertices_; idx++) {
        coords.push_back(teapotTexCoords[3 * idx] / 2);
        coords.push_back(teapotTexCoords[3* idx + 1] / 2);
    }
    glBufferData(GL_ARRAY_BUFFER,
                 kCoordElementCount * sizeof(float) * num_vertices_,
                 coords.data(), GL_STATIC_DRAW);
#endif
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    LoadTexture("Textures/android_pie.tga", assetMgr);
    GLint sampler = glGetUniformLocation(shader_param_.program_, "samplerObj");
    glUniform1i(sampler, 0);
}

/**
 * Render() function:
 *   enable states for rendering and reader a frame.
 *   For Texture, simply inform GL to stream texture coord from _texVbo
 */
void TexturedTeapotRender::Render() {
    // Bind the VBO
    glBindBuffer(GL_ARRAY_BUFFER, texVbo_);
    glVertexAttribPointer(ATTRIB_UV, 2, GL_FLOAT, GL_FALSE,
                          kCoordElementCount * sizeof(float),
                          BUFFER_OFFSET(0));
    glEnableVertexAttribArray(ATTRIB_UV);
    TeapotRenderer::Render();
}

/**
 * Unload()
 *    clean-up function. May get called from destructor too
 */
void TexturedTeapotRender::Unload() {
    if (texId_!= GL_INVALID_VALUE) {
        glDeleteTextures(1, &texId_);
        texId_ = GL_INVALID_VALUE;
    }
    if(texVbo_ != GL_INVALID_VALUE) {
        glDeleteBuffers(1, &texVbo_);
        texVbo_ = GL_INVALID_VALUE;
    }

    TeapotRenderer::Unload();
}
