/*
 * Copyright 2013 The Android Open Source Project
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

//--------------------------------------------------------------------------------
// Teapot Renderer.h
// Renderer for teapots
//--------------------------------------------------------------------------------
#ifndef _TEAPOTRENDERER_H
#define _TEAPOTRENDERER_H

//--------------------------------------------------------------------------------
// Include files
//--------------------------------------------------------------------------------
#include <jni.h>
#include <errno.h>

#include <vector>

#include <EGL/egl.h>
#include <GLES/gl.h>

#include <android/sensor.h>
#include <android/log.h>
#include <android_native_app_glue.h>
#include <android/native_window_jni.h>
#include <cpu-features.h>

#include "NDKSupport/NDKSupport.h"


#define BUFFER_OFFSET(i) ((char *)NULL + (i))

struct TEAPOT_VERTEX
{
    float fPos[3];
    float fNormal[3];
};

enum SHADER_ATTRIBUTES {
    ATTRIB_VERTEX, ATTRIB_NORMAL, ATTRIB_UV,
};

struct SHADER_PARAMS
{
    GLuint _program;
    GLuint _uiLight0;
    GLuint _uiMaterialDiffuse;
    GLuint _uiMaterialAmbient;
    GLuint _uiMaterialSpecular;

    GLuint _uiMatrixP;
    GLuint _uiMatrixView;
};

struct TEAPOT_MATERIALS
{
    float diffuse_color[ 3 ];
    float specular_color[ 4 ];
    float ambient_color[ 3 ];
};

class TeapotRenderer
{
    int32_t _iNumIndices;
    int32_t _iNumVertices;
    GLuint _ibo;
    GLuint _vbo;

    SHADER_PARAMS _shaderParam;
    bool loadShaders(SHADER_PARAMS* params, const char* strVsh, const char* strFsh);

     mat4 _mProjection;
     mat4 _mView;
     mat4 _mModel;

     tapCamera* _camera;
public:
    TeapotRenderer();
    virtual ~TeapotRenderer();
    void init();
    void render();
    void update(float dTime);
    bool bind(tapCamera* camera);
    void unload();

};

#endif

