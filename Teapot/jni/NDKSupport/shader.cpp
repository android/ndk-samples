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

#include "shader.h"
#include "JNIHelper.h"

#define DEBUG (1)

bool shader::compileShader(GLuint *shader, const GLenum type,
        const char *strFileName) {
    GLint status;
    const GLchar *source;

    std::vector<uint8_t> data;
    bool b = JNIHelper::readFile(strFileName, data);
    if (!b)
    {
        LOGI("Can not open a file:%s", strFileName);
        return false;
    }

    source = (GLchar *) &data[0];
    if (!source) {
        LOGI("Failed to load vertex shader:%s", strFileName);
        return false;
    }

    int32_t iSize = data.size();

    *shader = glCreateShader(type);
    glShaderSource(*shader, 1, &source, &iSize); //Not specifying 3rd parameter (size) could be troublesome..

    glCompileShader(*shader);

#if defined(DEBUG)
    GLint logLength;
    glGetShaderiv(*shader, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0) {
        GLchar *log = (GLchar *) malloc(logLength);
        glGetShaderInfoLog(*shader, logLength, &logLength, log);
        LOGI("Shader compile log:\n%s", log);
        free(log);
    }
#endif

    glGetShaderiv(*shader, GL_COMPILE_STATUS, &status);
    if (status == 0) {
        glDeleteShader(*shader);
        return false;
    }

    data.clear();
    return true;
}

bool shader::linkProgram(const GLuint prog) {
    GLint status;

    glLinkProgram(prog);

#if defined(DEBUG)
//  GLint logLength;
//  glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLength);
//  if (logLength > 0) {
//      GLchar *log = (GLchar *) malloc(logLength);
//      glGetProgramInfoLog(prog, logLength, &logLength, log);
//      LOGI("Program link log:\n%s", log);
//      free(log);
//  }
#endif

    glGetProgramiv(prog, GL_LINK_STATUS, &status);
    if (status == 0) {
        LOGI("Program link failed\n");
        return false;
    }

    return true;
}

bool shader::validateProgram(const GLuint prog) {
    GLint logLength, status;

    glValidateProgram(prog);
    glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0) {
        GLchar *log = (GLchar *) malloc(logLength);
        glGetProgramInfoLog(prog, logLength, &logLength, log);
        LOGI("Program validate log:\n%s", log);
        free(log);
    }

    glGetProgramiv(prog, GL_VALIDATE_STATUS, &status);
    if (status == 0)
        return false;

    return true;
}
