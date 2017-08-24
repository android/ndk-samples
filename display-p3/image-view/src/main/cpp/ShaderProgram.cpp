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
#include <vector>
#include "gldebug.h"
#include "android_debug.h"
#include "ShaderProgram.h"


static const char gVertexShader[] =
    "#version 300 es                         \n"
    "layout (location = 0) in vec4 vPosition;\n"
    "layout (location = 1) in vec2 vTexture;\n"
    "out vec2 vertex_tex;   \n"
    "void main() {          \n"
    "  gl_Position = vPosition;\n"
    "  vertex_tex = vTexture;              \n"
    "}\n";

static const char gFragmentShader[] =
    "#version 300 es         \n"
    "precision mediump float;\n"
    "in vec2 vertex_tex;     \n"
    "uniform sampler2D samplerObj; \n"
    "layout(location = 0) out vec4 oColor;      \n"
    "void main() { \n"
    "    oColor = texture(samplerObj, vertex_tex); \n"
    "} \n";

GLuint loadShader(GLenum shaderType, const char* pSource) {
  GLuint shader = glCreateShader(shaderType);
  if (shader) {
    glShaderSource(shader, 1, &pSource, NULL);
    glCompileShader(shader);
    GLint compiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
      GLint infoLen = 0;
      glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
      if (infoLen) {
        std::vector<char> buf(infoLen);
        if (buf.capacity() > 0) {
          glGetShaderInfoLog(shader, infoLen, NULL, buf.data());
          LOGE("Could not compile shader %d:\n%s\n", shaderType, buf.data());
        }
        glDeleteShader(shader);
        shader = 0;
      }
    }
  }
  return shader;
}

GLuint ShaderProgram::createProgram(void) {
  return createProgram(gVertexShader, gFragmentShader);
}
GLuint ShaderProgram::createProgram(const char* pVertexSource, const char* pFragmentSource) {
  GLuint vertexShader = loadShader(GL_VERTEX_SHADER, pVertexSource);
  if (!vertexShader) {
    return 0;
  }

  GLuint pixelShader = loadShader(GL_FRAGMENT_SHADER, pFragmentSource);
  if (!pixelShader) {
    return 0;
  }

  gProgram_ = glCreateProgram();
  if (gProgram_) {
    glAttachShader(gProgram_, vertexShader);
    glAttachShader(gProgram_, pixelShader);
    glLinkProgram(gProgram_);
    GLint linkStatus = GL_FALSE;
    glGetProgramiv(gProgram_, GL_LINK_STATUS, &linkStatus);
    if (linkStatus != GL_TRUE) {
      GLint bufLength = 0;
      glGetProgramiv(gProgram_, GL_INFO_LOG_LENGTH, &bufLength);
      if (bufLength) {
        std::vector<char> buf(bufLength);
        if (buf.capacity() > 0) {
          glGetProgramInfoLog(gProgram_, bufLength, NULL, buf.data());
          LOGE("Could not link program:\n%s\n", buf.data());
        }
      }
      glDeleteProgram(gProgram_);
      gProgram_ = 0;
      ASSERT(false, "Link Program failed");
    } else {
      gvPositionHandle_ = glGetAttribLocation(gProgram_, "vPosition");
      gvTxtHandle_  = glGetAttribLocation(gProgram_, "vTexture");
    }
  }

  return gProgram_;
}

GLint ShaderProgram::getSamplerLoc(void) {
  ASSERT(gProgram_, "Shader is not created");
  return glGetUniformLocation(gProgram_, "samplerObj");
}

