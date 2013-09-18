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
// TeapotRenderer.cpp
// Render a teapot
//--------------------------------------------------------------------------------

//--------------------------------------------------------------------------------
// Include files
//--------------------------------------------------------------------------------
#include "TeapotRenderer.h"

//--------------------------------------------------------------------------------
// Teapot model data
//--------------------------------------------------------------------------------
#include "teapot.inl"

//--------------------------------------------------------------------------------
// Ctor
//--------------------------------------------------------------------------------
TeapotRenderer::TeapotRenderer()
{

}

//--------------------------------------------------------------------------------
// Dtor
//--------------------------------------------------------------------------------
TeapotRenderer::~TeapotRenderer() {
    unload();
}

void TeapotRenderer::init()
{
    //Settings
    glFrontFace (GL_CCW);

    //Load shader
    loadShaders( &_shaderParam, "Shaders/VS_ShaderPlain.vsh", "Shaders/ShaderPlain.fsh" );

    //Create Index buffer
    _iNumIndices = sizeof(teapotIndices) / sizeof(teapotIndices[0]);
    glGenBuffers(1, &_ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
            sizeof( teapotIndices ) , teapotIndices,
            GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    //Create VBO
    _iNumVertices = sizeof(teapotPositions) / sizeof(teapotPositions[0]) / 3;
    int32_t iStride = sizeof(TEAPOT_VERTEX);
    int32_t iIndex = 0;
    TEAPOT_VERTEX* p = new TEAPOT_VERTEX[_iNumVertices];
    for( int32_t i = 0; i < _iNumVertices; ++i )
    {
        p[i].fPos[0] = teapotPositions[iIndex];
        p[i].fPos[1] = teapotPositions[iIndex+1];
        p[i].fPos[2] = teapotPositions[iIndex+2];

        p[i].fNormal[0] = teapotNormals[iIndex];
        p[i].fNormal[1] = teapotNormals[iIndex+1];
        p[i].fNormal[2] = teapotNormals[iIndex+2];
        iIndex += 3;
    }
    glGenBuffers(1, &_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, iStride * _iNumVertices,
            p, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    delete[] p;

    //Init Projection matrices
    int32_t viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    float fAspect = (float)viewport[2] / (float)viewport[3];

    const float CAM_NEAR = 5.f;
    const float CAM_FAR = 10000.f;
    bool bRotate = false;
    _mProjection = mat4::perspective(fAspect, 1.f,
            CAM_NEAR, CAM_FAR);
    _mModel = mat4::translation(0, 0, -15.f);

    mat4 mat = mat4::rotationX(M_PI / 3);
    _mModel = mat * _mModel;
}

void TeapotRenderer::unload()
{
    if (_vbo)
    {
        glDeleteBuffers(1, &_vbo);
        _vbo = 0;
    }

    if (_ibo)
    {
        glDeleteBuffers(1, &_ibo);
        _ibo = 0;
    }

    if (_shaderParam._program )
    {
        glDeleteProgram(_shaderParam._program);
        _shaderParam._program = 0;
    }
}

void TeapotRenderer::update(float fTime)
{
    const float CAM_X = 0.f;
    const float CAM_Y = 0.f;
    const float CAM_Z = 700.f;

    _mView = mat4::lookAt(vec3(CAM_X, CAM_Y, CAM_Z),
            vec3(0.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f));

    if( _camera )
    {
        _camera->update();
        _mView = _camera->getTransformMatrix() * _mView * _camera->getRotationMatrix() * _mModel;
    }
    else
    {
        _mView = _mView * _mModel;
    }
}

void TeapotRenderer::render()
{
    //
    // Feed Projection and Model View matrices to the shaders
    mat4 mVP = _mProjection * _mView;

    // Bind the VBO
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);

    int32_t iStride = sizeof(TEAPOT_VERTEX);
    // Pass the vertex data
    glVertexAttribPointer(ATTRIB_VERTEX, 3, GL_FLOAT, GL_FALSE, iStride,
            BUFFER_OFFSET( 0 ));
    glEnableVertexAttribArray(ATTRIB_VERTEX);

    glVertexAttribPointer(ATTRIB_NORMAL, 3, GL_FLOAT, GL_FALSE, iStride,
            BUFFER_OFFSET( 3 * sizeof(GLfloat) ));
    glEnableVertexAttribArray(ATTRIB_NORMAL);


    // Bind the IB
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibo);

    glUseProgram(_shaderParam._program);

    TEAPOT_MATERIALS material = {
            {1.0f, 0.5f, 0.5f},
            {1.0f, 1.0f, 1.0f, 10.f},
            {0.1f, 0.1f, 0.1f},
    };

    //Update uniforms
    glUniform4f(_shaderParam._uiMaterialDiffuse,
            material.diffuse_color[0],
            material.diffuse_color[1],
            material.diffuse_color[2],
            1.f);

    glUniform4f(_shaderParam._uiMaterialSpecular,
            material.specular_color[0],
            material.specular_color[1],
            material.specular_color[2],
            material.specular_color[3]);
    //
    //using glUniform3fv here was troublesome
    //
    glUniform3f(_shaderParam._uiMaterialAmbient,
            material.ambient_color[0],
            material.ambient_color[1],
            material.ambient_color[2]);

    glUniformMatrix4fv(_shaderParam._uiMatrixP, 1, GL_FALSE,
                    mVP.ptr());
    glUniformMatrix4fv(_shaderParam._uiMatrixView, 1, GL_FALSE,
        _mView.ptr());
    glUniform3f(_shaderParam._uiLight0, 100.f, -200.f, -600.f);

    glDrawElements(GL_TRIANGLES, _iNumIndices, GL_UNSIGNED_SHORT,
                    BUFFER_OFFSET(0));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

bool TeapotRenderer::loadShaders(SHADER_PARAMS* params, const char* strVsh, const char* strFsh)
{
    GLuint program;
    GLuint vertShader, fragShader;
    char *vertShaderPathname, *fragShaderPathname;

    // Create shader program
    program = glCreateProgram();
    LOGI("Created Shader %d", program);

    // Create and compile vertex shader
    if (!shader::compileShader(&vertShader, GL_VERTEX_SHADER, strVsh)) {
        LOGI("Failed to compile vertex shader");
        glDeleteProgram(program);
        return false;
    }

    // Create and compile fragment shader
    if (!shader::compileShader(&fragShader, GL_FRAGMENT_SHADER, strFsh)) {
        LOGI("Failed to compile fragment shader");
        glDeleteProgram(program);
        return false;
    }

    // Attach vertex shader to program
    glAttachShader(program, vertShader);

    // Attach fragment shader to program
    glAttachShader(program, fragShader);

    // Bind attribute locations
    // this needs to be done prior to linking
    glBindAttribLocation(program, ATTRIB_VERTEX, "myVertex");
    glBindAttribLocation(program, ATTRIB_NORMAL, "myNormal");
    glBindAttribLocation(program, ATTRIB_UV, "myUV");

    // Link program
    if (!shader::linkProgram(program)) {
        LOGI("Failed to link program: %d", program);

        if (vertShader) {
            glDeleteShader(vertShader);
            vertShader = 0;
        }
        if (fragShader) {
            glDeleteShader(fragShader);
            fragShader = 0;
        }
        if (program) {
            glDeleteProgram(program);
        }

        return false;
    }

    // Get uniform locations
    params->_uiMatrixP = glGetUniformLocation(program, "uPMatrix");
    params->_uiMatrixView = glGetUniformLocation(program, "uMVMatrix");

    params->_uiLight0 = glGetUniformLocation(program, "vLight0");
    params->_uiMaterialDiffuse = glGetUniformLocation(program,
            "vMaterialDiffuse");
    params->_uiMaterialAmbient = glGetUniformLocation(program,
            "vMaterialAmbient");
    params->_uiMaterialSpecular = glGetUniformLocation(program,
            "vMaterialSpecular");

    // Release vertex and fragment shaders
    if (vertShader)
        glDeleteShader(vertShader);
    if (fragShader)
        glDeleteShader(fragShader);

    params->_program = program;
    return true;
}

bool TeapotRenderer::bind(tapCamera* camera)
{
    _camera = camera;
    return true;
}

