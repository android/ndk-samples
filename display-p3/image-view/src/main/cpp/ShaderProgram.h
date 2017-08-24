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
#ifndef __SHADER_PROGRAM_H__
#define __SHADER_PROGRAM_H__

#include <GLES3/gl32.h>

class ShaderProgram {
public:
  ShaderProgram() {};
  GLuint createProgram(void);
  GLuint createProgram(const char* pVertexSource, const char* pFragmentSource);
  GLuint getAttribLocation() const { return gvPositionHandle_; }
  GLuint getAttribLocationTex() const { return gvTxtHandle_; }
  GLuint getProgram() const { return gProgram_; }
  GLint  getSamplerLoc(void);
private:
  GLuint gProgram_;
  GLuint gvPositionHandle_;
  GLuint gvTxtHandle_;
};

#endif // __SHADER_PROGRAM_H__
