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

extern float  teapotTexCoords[];
constexpr  int32_t kCoordElementCount = (TILED_TEXTURE ? 3 : 2);


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
 * Report type of teapot we are rendering. This is the only place
 * to decide what kind of teapot to render.
 *
 * @return
 *   GL_TEXTURE_CUBE_MAP if you want to render cubemaped teapot
 *   GL_TEXTURE_2D if just to render a 2D textured teapot
 *   GL_INVALID_VALUE no texture for teapot
 */
GLint TexturedTeapotRender::GetTextureType(void) {
    return
            GL_TEXTURE_CUBE_MAP;
//            GL_TEXTURE_2D;
//            GL_INVALID_VALUE;
}

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

    GLint type = GetTextureType();
    if(type == GL_INVALID_VALUE) {
        // Plain teapot no texture
        return;
    }

    // load texture coordinator for 2D texture. Cubemap texture uses world space normal
    // to sample cubemap.
    if(type == GL_TEXTURE_2D) {
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
        glVertexAttribPointer(ATTRIB_UV, 2, GL_FLOAT, GL_FALSE,
                              kCoordElementCount * sizeof(float),
                              BUFFER_OFFSET(0));
        glEnableVertexAttribArray(ATTRIB_UV);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    // Need flip Y, so as top/bottom image
    std::vector<std::string> textures {
            std::string("Textures/right.tga"),  // GL_TEXTURE_CUBE_MAP_POSITIVE_X
            std::string("Textures/left.tga"),   // GL_TEXTURE_CUBE_MAP_NEGATIVE_X
            std::string("Textures/bottom.tga"), // GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
            std::string("Textures/top.tga"),    // GL_TEXTURE_CUBE_MAP_POSITIVE_Y
            std::string("Textures/front.tga"),  // GL_TEXTURE_CUBE_MAP_POSITIVE_Z
            std::string("Textures/back.tga")    // GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
    };

    if(type == GL_TEXTURE_2D) {
        textures[0] = std::string("Textures/front.tga");
    }

    texObj_ = Texture::Create(type, textures, assetMgr);
    assert(texObj_);

    std::vector<std::string> samplers;
    std::vector<GLint> units;
    texObj_->GetActiveSamplerInfo(samplers, units);
    for(size_t idx = 0; idx < samplers.size(); idx++) {
        GLint sampler = glGetUniformLocation(shader_param_.program_,
                                             samplers[idx].c_str());
        glUniform1i(sampler, units[idx]);
    }

    texObj_->Activate();
}

/**
 * Render() function:
 *   enable states for rendering and reader a frame.
 *   For Texture, simply inform GL to stream texture coord from _texVbo
 */
void TexturedTeapotRender::Render() {
    TeapotRenderer::Render();
}

/**
 * Unload()
 *    clean-up function. May get called from destructor too
 */
void TexturedTeapotRender::Unload() {
    TeapotRenderer::Unload();
    if(texVbo_ != GL_INVALID_VALUE) {
        glDeleteBuffers(1, &texVbo_);
        texVbo_ = GL_INVALID_VALUE;
    }
    if(texObj_) {
        Texture::Delete(texObj_);
        texObj_ = nullptr;
    }
}
