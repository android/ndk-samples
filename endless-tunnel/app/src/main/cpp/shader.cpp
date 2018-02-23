/*
 * Copyright (C) Google Inc.
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
#include "common.hpp"
#include "indexbuf.hpp"
#include "shader.hpp"
#include "vertexbuf.hpp"

Shader::Shader() {
    mVertShaderH = mFragShaderH = mProgramH = 0;
    mMVPMatrixLoc = -1;
    mPositionAttribLoc = -1;
    mPreparedVertexBuf = NULL;
}

Shader::~Shader() {
    if (mVertShaderH) {
        glDeleteShader(mVertShaderH);
        mVertShaderH = 0;
    }
    if (mFragShaderH) {
        glDeleteShader(mFragShaderH);
        mFragShaderH = 0;
    }
    if (mProgramH) {
        glDeleteProgram(mProgramH);
        mProgramH = 0;
    }
}

static void _printShaderLog(GLuint shader) {
   char buf[2048];
   memset(buf, 0, sizeof(buf));
   LOGE("*** Getting info log for shader %u", shader);
   glGetShaderInfoLog(shader, sizeof(buf) - 1, NULL, buf);
   LOGE("*** Info log:\n%s", buf);
}

static void _printProgramLog(GLuint program) {
   char buf[2048];
   memset(buf, 0, sizeof(buf));
   LOGE("*** Getting info log for program %u", program);
   glGetProgramInfoLog(program, sizeof(buf) - 1, NULL, buf);
   LOGE("*** Info log:\n%s", buf);
}


void Shader::Compile() {
    const char *vsrc = 0, *fsrc = 0;
    GLint status = 0;

    LOGD("Compiling shader.");
    LOGD("Shader name: %s", GetShaderName());

    vsrc = GetVertShaderSource();
    fsrc = GetFragShaderSource();

    mVertShaderH = glCreateShader(GL_VERTEX_SHADER);
    mFragShaderH = glCreateShader(GL_FRAGMENT_SHADER);
    if (!mVertShaderH || !mFragShaderH) {
        LOGE("*** Failed to create shader.");
        ABORT_GAME;
    }
    glShaderSource(mVertShaderH, 1, &vsrc, NULL);
    glCompileShader(mVertShaderH);
    glGetShaderiv(mVertShaderH, GL_COMPILE_STATUS, &status);
    if (status == 0) {
        LOGE("*** Vertex shader compilation failed.");
        _printShaderLog(mVertShaderH);
        ABORT_GAME;
    }
    LOGD("Vertex shader compilation succeeded.");

    glShaderSource(mFragShaderH, 1, &fsrc, NULL);
    glCompileShader(mFragShaderH);
    glGetShaderiv(mFragShaderH, GL_COMPILE_STATUS, &status);
    if (status == 0) {
        LOGE("*** Fragment shader compilation failed, %d", status);
        _printShaderLog(mFragShaderH);
        ABORT_GAME;
    }
    LOGD("Fragment shader compilation succeeded.");

    mProgramH = glCreateProgram();
    if (!mProgramH) {
        LOGE("*** Failed to create program");
        _printProgramLog(mProgramH);
        ABORT_GAME;
    }

    glAttachShader(mProgramH, mVertShaderH);
    glAttachShader(mProgramH, mFragShaderH);
    glLinkProgram(mProgramH);
    glGetProgramiv(mProgramH, GL_LINK_STATUS, &status);
    if (status == 0) {
        LOGE("*** Shader program link failed, %d", status);
        _printProgramLog(mProgramH);
        ABORT_GAME;
    }
    LOGD("Program linking succeeded.");

    glUseProgram(mProgramH);
    mMVPMatrixLoc = glGetUniformLocation(mProgramH, "u_MVP");
    if (mMVPMatrixLoc < 0) {
        LOGE("*** Couldn't get shader's u_MVP matrix location from shader.");
        ABORT_GAME;
    }
    mPositionAttribLoc = glGetAttribLocation(mProgramH, "a_Position");
    if (mPositionAttribLoc < 0) {
       LOGE("*** Couldn't get shader's a_Position attribute location.");
       ABORT_GAME;
    }
    LOGD("Shader compilation/linking successful.");
    glUseProgram(0);
}

void Shader::BindShader() {
    if (mProgramH == 0) {
        LOGW("!!! WARNING: attempt to use shader before compiling.");
        LOGW("!!! Compiling now. Shader: %s", GetShaderName());
        Compile();
    }
    glUseProgram(mProgramH);
}

void Shader::UnbindShader() {
    glUseProgram(0);
}

// To be called by child classes only.
void Shader::PushMVPMatrix(glm::mat4 *mat) {
    MY_ASSERT(mMVPMatrixLoc >= 0);
    glUniformMatrix4fv(mMVPMatrixLoc, 1, GL_FALSE, glm::value_ptr(*mat));
}

// To be called by child classes only.
void Shader::PushPositions(int vbo_offset, int stride) {
   MY_ASSERT(mPositionAttribLoc >= 0);
   glVertexAttribPointer(mPositionAttribLoc, 3, GL_FLOAT, GL_FALSE, stride,
           BUFFER_OFFSET(vbo_offset));
   glEnableVertexAttribArray(mPositionAttribLoc);
}

void Shader::BeginRender(VertexBuf *vbuf) {
    // Activate shader
    BindShader();

    // bind geometry's VBO
    vbuf->BindBuffer();

    // push positions to shader
    PushPositions(vbuf->GetPositionsOffset(), vbuf->GetStride());

    // store geometry
    mPreparedVertexBuf = vbuf;
}

void Shader::Render(IndexBuf *ibuf, glm::mat4* mvpMat) {
    MY_ASSERT(mPreparedVertexBuf != NULL);

    // push MVP matrix to shader
    PushMVPMatrix(mvpMat);

    if (ibuf) {
        // draw with index buffer
        ibuf->BindBuffer();
        glDrawElements(mPreparedVertexBuf->GetPrimitive(), ibuf->GetCount(), GL_UNSIGNED_SHORT,
                BUFFER_OFFSET(0));
        ibuf->UnbindBuffer();
    } else {
        // draw straight from vertex buffer
        glDrawArrays(mPreparedVertexBuf->GetPrimitive(), 0, mPreparedVertexBuf->GetCount());
    }
}

void Shader::EndRender() {
    if (mPreparedVertexBuf) {
        mPreparedVertexBuf->UnbindBuffer();
        mPreparedVertexBuf = NULL;
    }
}




TrivialShader::TrivialShader() : Shader() {
    mColorLoc = -1;
    mTintLoc = -1;
    mTint[0] = mTint[1] = mTint[2] = 1.0f; // white
}

TrivialShader::~TrivialShader() {
}

void TrivialShader::Compile() {
    Shader::Compile();
    BindShader();
    mColorLoc = glGetAttribLocation(mProgramH, "a_Color");
    if (mColorLoc < 0) {
        LOGE("*** Couldn't get color attrib location from shader.");
        ABORT_GAME;
    }
    mTintLoc = glGetUniformLocation(mProgramH, "u_Tint");
    if (mTintLoc < 0) {
        LOGE("*** Couldn't get tint uniform location from shader.");
        ABORT_GAME;
    }
    UnbindShader();
}

const char* TrivialShader::GetVertShaderSource() {
    return "uniform mat4 u_MVP;            \n"
           "uniform vec4 u_Tint;           \n"
           "attribute vec4 a_Position;     \n"
           "attribute vec4 a_Color;        \n"
           "varying vec4 v_Color;          \n"
           "void main()                    \n"
           "{                              \n"
           "   v_Color = a_Color * u_Tint; \n"
           "   gl_Position = u_MVP         \n"
           "               * a_Position;   \n"
           "}                              \n";
}

const char* TrivialShader::GetFragShaderSource() {
    return "precision mediump float;       \n"
           "varying vec4 v_Color;          \n"
           "void main()                    \n"
           "{                              \n"
           "   gl_FragColor = v_Color;     \n"
           "}";
}

int TrivialShader::GetColorAttribLoc() {
    return mColorLoc;
}

const char* TrivialShader::GetShaderName() {
    return "TrivialShader";
}

void TrivialShader::ResetTintColor() {
    SetTintColor(1.0f, 1.0f, 1.0f);
}

void TrivialShader::SetTintColor(float r, float g, float b) {
    mTint[0] = r;
    mTint[1] = g;
    mTint[2] = b;

    if (mPreparedVertexBuf) {
        // we are in the middle of rendering, so push the new tint color to
        // the shader right away.
        glUniform4f(mTintLoc, mTint[0], mTint[1], mTint[2], 1.0f);
    }
}

void TrivialShader::BeginRender(VertexBuf *geom) {
    // let superclass do the basic work
    Shader::BeginRender(geom);

    // this shader requires colors, so make sure we have them.
    MY_ASSERT(geom->HasColors());
    MY_ASSERT(mColorLoc >= 0);

    // push colors to shader
    glVertexAttribPointer(mColorLoc, 3, GL_FLOAT, GL_FALSE, geom->GetStride(),
            BUFFER_OFFSET(geom->GetColorsOffset()));
    glEnableVertexAttribArray(mColorLoc);

    // push tint color to shader
    MY_ASSERT(mTintLoc >= 0);
    glUniform4f(mTintLoc, mTint[0], mTint[1], mTint[2], 1.0f);
}


