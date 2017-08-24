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
#include <memory>
#include "ImageViewEngine.h"

/*
 * Create Rendering Context
 */
bool ImageViewEngine::InitializeDisplay(void) {

  EnableWelcomeUI();

  bool status = CreateWideColorCtx();
  ASSERT(status, "CreateWideColorContext() failed");

  status = program_.createProgram();
  ASSERT(status, "CreateShaderProgram Failed");

  status = CreateTextures();
  ASSERT(status, "LoadTextures() Failed")

  // Other GL States
  glDisable(GL_DEPTH_TEST);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glViewport(0, 0, renderTargetWidth_, renderTargetHeight_);

  EnableRenderUI();
  return true;
}

bool ImageViewEngine::GetAnimationStatus(void) {
  return animating_;
}
void ImageViewEngine::EnableAnimation(bool enable) {
  animating_ = enable;
}

/*
 * Draw quad(s) to view texture
 */
void ImageViewEngine::DrawFrame(void) {
  if (display_ == NULL) {
    return;
  }

  const GLfloat leftQuadVertices[] = {
      -1.f, -1.0f,  0.0f, 1.0f,
      0.0f, -1.0f,  1.0f, 1.0f,
      0.0f,  1.0f,  1.0f, 0.0f,
      -1.0f, 1.0f,  0.0f, 0.0f };
  const GLfloat rightQuadVertices[] = {
      0.0f, -1.0f,  0.0f, 1.0f,
      1.0f, -1.0f,  1.0f, 1.0f,
      1.0f,  1.0f,  1.0f, 0.0f,
      0.0f, 1.0f,  0.0f, 0.0f };

  // Just fill the screen with a color.
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  glUseProgram(program_.getProgram());

  glVertexAttribPointer(program_.getAttribLocation(),
                        2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, leftQuadVertices);
  glEnableVertexAttribArray(program_.getAttribLocation());

  glVertexAttribPointer(program_.getAttribLocationTex(),
                        2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, leftQuadVertices + 2);
  glEnableVertexAttribArray(program_.getAttribLocationTex());
  int32_t texIdx = textureIdx_;
  if(renderModeBits_ & RENDERING_P3) {
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, textures_[texIdx]->P3TexId());
    glUniform1i(program_.getSamplerLoc(), 0);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
  }
  if (renderModeBits_ & RENDERING_SRGB) {
    glVertexAttribPointer(program_.getAttribLocation(),
                          2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4,
                          rightQuadVertices);
    glVertexAttribPointer(program_.getAttribLocationTex(),
                          2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4,
                          rightQuadVertices + 2);
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, textures_[texIdx]->SRGBATexId());
    glUniform1i(program_.getSamplerLoc(), 1);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
  }

  eglSwapBuffers(display_, surface_);
}

/**
 * Tear down the EGL context currently associated with the display.
 */
void ImageViewEngine::TerminateDisplay(void) {

  animating_ = 0;

  if(display_ == EGL_NO_DISPLAY)
    return;

  DestroyWideColorCtx();

  glDeleteProgram(program_.getProgram());
  DeleteTextures();
}

/*
 * Initialize the application engine
 */
ImageViewEngine::ImageViewEngine(struct android_app* app) :
    app_(app),
    animating_(0),
    textureIdx_(0) {
  textures_.resize(0);
  renderModeBits_ = RENDERING_P3 | RENDERING_SRGB;
  eglContext_ = EGL_NO_CONTEXT;
  surface_ = EGL_NO_SURFACE;
  display_ = EGL_NO_DISPLAY;
}
