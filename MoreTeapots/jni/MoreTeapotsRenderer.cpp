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
// MoreTeapotsRenderer.cpp
// Render teapots
//--------------------------------------------------------------------------------

//--------------------------------------------------------------------------------
// Include files
//--------------------------------------------------------------------------------
#include "MoreTeapotsRenderer.h"

//--------------------------------------------------------------------------------
// Teapot model data
//--------------------------------------------------------------------------------
#include "teapot.inl"

//--------------------------------------------------------------------------------
// Ctor
//--------------------------------------------------------------------------------
MoreTeapotsRenderer::MoreTeapotsRenderer() : _bGeometryInstancingSupport( false )
{

}

//--------------------------------------------------------------------------------
// Dtor
//--------------------------------------------------------------------------------
MoreTeapotsRenderer::~MoreTeapotsRenderer() {
    unload();
}

//--------------------------------------------------------------------------------
// Init
//--------------------------------------------------------------------------------
void MoreTeapotsRenderer::init( const int32_t numX, const int32_t numY, const int32_t numZ )
{
    if( GLContext::getInstance()->getGLVersion() >= 3.0 )
    {
        _bGeometryInstancingSupport = true;
    }
    else if( GLContext::getInstance()->checkExtension("GL_NV_draw_instanced")
            && GLContext::getInstance()->checkExtension("GL_NV_uniform_buffer_object") )
    {
        LOGI( "Supported via extension!" );
        //_bGeometryInstancingSupport = true;
        //_bARBSupport = true; //Need to patch shaders
        //Currently this has been disabled
    }

    //Settings
    glFrontFace (GL_CCW);

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
    _iX = numX;
    _iY = numY;
    _iZ = numZ;
    _mModels.reserve( _iX * _iY * _iZ );

    updateViewport();

    float fTotalWidth = 500.f;
    float fGapX = fTotalWidth / (_iX - 1);
    float fGapY = fTotalWidth / (_iY - 1);
    float fGapZ = fTotalWidth / (_iZ - 1);
    float fOffsetX = -fTotalWidth/ 2.f;
    float fOffsetY = -fTotalWidth/ 2.f;
    float fOffsetZ = -fTotalWidth/ 2.f;

    for( int32_t iX = 0; iX < _iX; ++iX )
        for( int32_t iY = 0; iY < _iY; ++iY )
            for( int32_t iZ = 0; iZ < _iZ; ++iZ )
            {
                _mModels.push_back( mat4::translation( iX * fGapX + fOffsetX,
                        iY * fGapY + fOffsetY,
                        iZ * fGapZ + fOffsetZ) );
                _mColors.push_back(
                        vec3( random() / float(RAND_MAX * 1.1),
                                random() / float(RAND_MAX * 1.1),
                                random() / float(RAND_MAX * 1.1)
                        ) );

                float fX = random() / float(RAND_MAX) - 0.5f;
                float fY = random() / float(RAND_MAX) - 0.5f;
                _mVecRotation.push_back(
                        vec2( fX * 0.05f, fY * 0.05f ) );
                _mCurrentRotation.push_back(
                        vec2( fX * M_PI, fY * M_PI ) );
            }

    if( _bGeometryInstancingSupport )
    {
        //
        //Create parameter dictionary for shader patch
        std::map< std::string, std::string> param;
        param[ std::string( "%NUM_TEAPOT%" ) ] = toString( _iX * _iY * _iZ );
        param[ std::string( "%LOCATION_VERTEX%" ) ] = toString( ATTRIB_VERTEX );
        param[ std::string( "%LOCATION_NORMAL%" ) ] = toString( ATTRIB_NORMAL );
        if( _bARBSupport )
            param[ std::string( "%ARB%" ) ] = std::string( "ARB" );
        else
            param[ std::string( "%ARB%" ) ] = std::string( "" );

        //Load shader
        bool b = loadShadersES3( &_shaderParam, "Shaders/VS_ShaderPlainES3.vsh", "Shaders/ShaderPlainES3.fsh", param );
        if( b )
        {
            //
            //Create uniform buffer
            //
            GLuint bindingPoint = 1;
            GLuint blockIndex;
            blockIndex = glGetUniformBlockIndex( _shaderParam._program, "ParamBlock" );
            glUniformBlockBinding( _shaderParam._program, blockIndex, bindingPoint );

            //Retrieve array stride value
            int32_t iNumIndices;
            glGetActiveUniformBlockiv( _shaderParam._program, blockIndex,
                    GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &iNumIndices );
            GLint i[iNumIndices];
            GLint stride[iNumIndices];
            glGetActiveUniformBlockiv( _shaderParam._program, blockIndex,
                    GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, i );
            glGetActiveUniformsiv( _shaderParam._program, iNumIndices, (GLuint*)i, GL_UNIFORM_ARRAY_STRIDE, stride );

            _uboMatrixStride = stride[ 0 ] / sizeof(float);
            _uboVectorStride = stride[ 2 ] / sizeof(float);

            glGenBuffers(1, &_ubo);
            glBindBuffer(GL_UNIFORM_BUFFER, _ubo);
            glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, _ubo);

            //Store color value which wouldn't be updated every frame
            int32_t iSize =  _iX * _iY * _iZ *
                    (_uboMatrixStride+_uboMatrixStride+_uboVectorStride); //mat4 + mat4 + vec3 + 1 stride
            float* pBuffer = new float[iSize];
            float* pColor = pBuffer + _iX * _iY * _iZ * _uboMatrixStride*2;
            for( int32_t i = 0; i < _iX * _iY * _iZ; ++i )
            {
                memcpy(pColor, &_mColors[ i ], 3 * sizeof(float));
                pColor += _uboVectorStride;    //Assuming std140 layout which is 4 DWORD stride for vectors
            }

            glBufferData(GL_UNIFORM_BUFFER, iSize * sizeof(float), pBuffer, GL_DYNAMIC_DRAW);
            delete[] pBuffer;
        }
        else
        {
            LOGI("Shader compilation failed!! Falls back to ES2.0 pass"); //This happens some devices.
            _bGeometryInstancingSupport = false;
            //Load shader for GLES2.0
            loadShaders( &_shaderParam, "Shaders/VS_ShaderPlain.vsh", "Shaders/ShaderPlain.fsh" );
        }
    }
    else
    {
        //Load shader for GLES2.0
        loadShaders( &_shaderParam, "Shaders/VS_ShaderPlain.vsh", "Shaders/ShaderPlain.fsh" );
    }
}

void MoreTeapotsRenderer::updateViewport()
{
    int32_t viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    float fAspect = (float)viewport[2] / (float)viewport[3];

    const float CAM_NEAR = 5.f;
    const float CAM_FAR = 10000.f;
    bool bRotate = false;
    _mProjection = mat4::perspective(fAspect, 1.f,
            CAM_NEAR, CAM_FAR);
}

//--------------------------------------------------------------------------------
// Unload
//--------------------------------------------------------------------------------
void MoreTeapotsRenderer::unload()
{
    if (_vbo)
    {
        glDeleteBuffers(1, &_vbo);
        _vbo = 0;
    }
    if (_ubo)
    {
        glDeleteBuffers(1, &_ubo);
        _ubo = 0;
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

//--------------------------------------------------------------------------------
// Update
//--------------------------------------------------------------------------------
void MoreTeapotsRenderer::update(float fTime)
{
    const float CAM_X = 0.f;
    const float CAM_Y = 0.f;
    const float CAM_Z = 2000.f;

    _mView = mat4::lookAt(vec3(CAM_X, CAM_Y, CAM_Z),
            vec3(0.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f));

    if( _camera )
    {
        _camera->update();
        _mView = _camera->getTransformMatrix() * _mView * _camera->getRotationMatrix();
    }
}

//--------------------------------------------------------------------------------
// Render
//--------------------------------------------------------------------------------
void MoreTeapotsRenderer::render()
{

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
            {1.0f, 1.0f, 1.0f, 10.f},
            {0.1f, 0.1f, 0.1f},
    };

    //Update uniforms
    //
    //using glUniform3fv here was troublesome..
    //
    glUniform4f(_shaderParam._uiMaterialSpecular,
            material.specular_color[0],
            material.specular_color[1],
            material.specular_color[2],
            material.specular_color[3]);
    glUniform3f(_shaderParam._uiMaterialAmbient,
            material.ambient_color[0],
            material.ambient_color[1],
            material.ambient_color[2]);

    glUniform3f(_shaderParam._uiLight0, 100.f, -200.f, -600.f);

    if( _bGeometryInstancingSupport )
    {
        //
        //Geometry instancing, new feature in GLES3.0
        //

        //Update UBO
        glBindBuffer(GL_UNIFORM_BUFFER, _ubo);
        float* p = (float*)glMapBufferRange(GL_UNIFORM_BUFFER,
                    0, _iX * _iY * _iZ * (_uboMatrixStride * 2) * sizeof(float),
                    GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
        float* pMVPMat = p;
        float* pMVMat = p + _iX * _iY * _iZ * _uboMatrixStride;
        for( int32_t i = 0; i < _iX * _iY * _iZ; ++i )
        {
            //Rotation
            float fX, fY;
            _mCurrentRotation[ i ] += _mVecRotation[ i ];
            _mCurrentRotation[ i ].value( fX, fY );
            mat4 mRotation = mat4::rotationX( fX ) * mat4::rotationY( fY );

            // Feed Projection and Model View matrices to the shaders
            mat4 mV = _mView * _mModels[ i ] * mRotation;
            mat4 mVP = _mProjection * mV;

            memcpy(pMVPMat, mVP.ptr(), sizeof(mV) );
            pMVPMat += _uboMatrixStride;

            memcpy(pMVMat, mV.ptr(), sizeof(mV) );
            pMVMat += _uboMatrixStride;
        }
        glUnmapBuffer(GL_UNIFORM_BUFFER);

        //Instanced rendering
        glDrawElementsInstanced(GL_TRIANGLES, _iNumIndices, GL_UNSIGNED_SHORT,
                            BUFFER_OFFSET(0), _iX * _iY * _iZ);

    }
    else
    {
        //Regular rendering pass
        for( int32_t i = 0; i < _iX * _iY * _iZ; ++i )
        {
            //Set diffuse
            float fX, fY, fZ;
            _mColors[ i ].value( fX, fY, fZ );
            glUniform4f(_shaderParam._uiMaterialDiffuse, fX, fY, fZ,
                    1.f);

            //Rotation
            _mCurrentRotation[ i ] += _mVecRotation[ i ];
            _mCurrentRotation[ i ].value( fX, fY );
            mat4 mRotation = mat4::rotationX( fX ) * mat4::rotationY( fY );

            // Feed Projection and Model View matrices to the shaders
            mat4 mV = _mView * _mModels[ i ] * mRotation;
            mat4 mVP = _mProjection * mV;
            glUniformMatrix4fv(_shaderParam._uiMatrixP, 1, GL_FALSE,
                            mVP.ptr());
            glUniformMatrix4fv(_shaderParam._uiMatrixView, 1, GL_FALSE,
                mV.ptr());

            glDrawElements(GL_TRIANGLES, _iNumIndices, GL_UNSIGNED_SHORT,
                                BUFFER_OFFSET(0));

        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

//--------------------------------------------------------------------------------
// LoadShaders
//--------------------------------------------------------------------------------
bool MoreTeapotsRenderer::loadShaders(SHADER_PARAMS* params, const char* strVsh, const char* strFsh)
{
    //
    //Shader load for GLES2
    //In GLES2.0, shader attribute locations need to be explicitly specified before linking
    //
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

bool MoreTeapotsRenderer::loadShadersES3(SHADER_PARAMS* params,
        const char* strVsh, const char* strFsh,
        std::map<std::string, std::string>&shaderParams )
{
    //
    //Shader load for GLES3
    //In GLES3.0, shader attribute index can be described in a shader code directly with layout() attribute
    //
    GLuint program;
    GLuint vertShader, fragShader;
    char *vertShaderPathname, *fragShaderPathname;

    // Create shader program
    program = glCreateProgram();
    LOGI("Created Shader %d", program);

    // Create and compile vertex shader
    if (!shader::compileShader(&vertShader, GL_VERTEX_SHADER, strVsh, shaderParams)) {
        LOGI("Failed to compile vertex shader");
        glDeleteProgram(program);
        return false;
    }

    // Create and compile fragment shader
    if (!shader::compileShader(&fragShader, GL_FRAGMENT_SHADER, strFsh, shaderParams)) {
        LOGI("Failed to compile fragment shader");
        glDeleteProgram(program);
        return false;
    }

    // Attach vertex shader to program
    glAttachShader(program, vertShader);

    // Attach fragment shader to program
    glAttachShader(program, fragShader);

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
    params->_uiLight0 = glGetUniformLocation(program, "vLight0");
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

//--------------------------------------------------------------------------------
// Bind
//--------------------------------------------------------------------------------
bool MoreTeapotsRenderer::bind(tapCamera* camera)
{
    _camera = camera;
    return true;
}

//--------------------------------------------------------------------------------
// Helper functions
//--------------------------------------------------------------------------------
std::string MoreTeapotsRenderer::toString( const int32_t i )
{
    char str[64];
    snprintf( str, sizeof( str ), "%d", i );
    return std::string( str );
}

