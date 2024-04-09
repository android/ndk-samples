/*
 * Copyright (C) 2009 The Android Open Source Project
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

// OpenGL ES 3.0 demo for efficient multisampling

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <android/log.h>
#include <jni.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "gl3stub.h"

#define LOG_TAG "libgl2jni"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

static void printGLString(const char* name, GLenum s) {
  const char* v = (const char*)glGetString(s);
  LOGI("GL %s = %s\n", name, v);
}

static void checkGlError(const char* op) {
  for (GLint error = glGetError(); error; error = glGetError()) {
    LOGI("after %s() glError (0x%x)\n", op, error);
  }
}

auto gVertexShader =
    "#version 300 es\n"
    "in vec4 vPosition;\n"
    "void main() {\n"
    "  gl_Position = vPosition;\n"
    "}\n";

auto gFragmentShader =
    "#version 300 es\n"
    "precision mediump float;\n"
    "layout(location = 0) out vec4 outColor1;\n"
    "layout(location = 1) out vec4 outColor2;\n"
    "layout(location = 2) out vec4 outColor3;\n"
    "layout(location = 3) out vec4 outColor4;\n"
    "void main() {\n"
    "  outColor1 = vec4(1.0, 0.0, 0.0, 1.0);\n"
    "  outColor2 = vec4(1.0, 0.0, 0.0, 1.0);\n"
    "  outColor3 = vec4(1.0, 0.0, 0.0, 1.0);\n"
    "  outColor4 = vec4(1.0, 0.0, 0.0, 1.0);\n"
    "}\n";

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
        char* buf = (char*)malloc(infoLen);
        if (buf) {
          glGetShaderInfoLog(shader, infoLen, NULL, buf);
          LOGE("Could not compile shader %d:\n%s\n", shaderType, buf);
          free(buf);
        }
        glDeleteShader(shader);
        shader = 0;
      }
    }
  }
  return shader;
}

GLuint createProgram(const char* pVertexSource, const char* pFragmentSource) {
  GLuint vertexShader = loadShader(GL_VERTEX_SHADER, pVertexSource);
  if (!vertexShader) {
    return 0;
  }

  GLuint pixelShader = loadShader(GL_FRAGMENT_SHADER, pFragmentSource);
  if (!pixelShader) {
    return 0;
  }

  GLuint program = glCreateProgram();
  if (program) {
    glAttachShader(program, vertexShader);
    checkGlError("glAttachShader");
    glAttachShader(program, pixelShader);
    checkGlError("glAttachShader");
    glLinkProgram(program);
    GLint linkStatus = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    if (linkStatus != GL_TRUE) {
      GLint bufLength = 0;
      glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
      if (bufLength) {
        char* buf = (char*)malloc(bufLength);
        if (buf) {
          glGetProgramInfoLog(program, bufLength, NULL, buf);
          LOGE("Could not link program:\n%s\n", buf);
          free(buf);
        }
      }
      glDeleteProgram(program);
      program = 0;
    }
  }
  return program;
}

GLuint gProgram;
GLuint gvPositionHandle;

GLuint gVertexBuffer;

static constexpr uint32_t kImageWidth = 1024;
static constexpr uint32_t kImageHeight = 2048;
static constexpr uint32_t kImageCount = 4;
GLuint gImages[kImageCount];
GLuint gFramebuffer;

int gWindowWidth = 1, gWindowHeight = 1;

bool setupGraphics(int w, int h) {
  gWindowWidth = w;
  gWindowHeight = h;

  printGLString("Version", GL_VERSION);
  printGLString("Vendor", GL_VENDOR);
  printGLString("Renderer", GL_RENDERER);
  printGLString("Extensions", GL_EXTENSIONS);

  LOGI("setupGraphics(%d, %d)", w, h);
  gProgram = createProgram(gVertexShader, gFragmentShader);
  if (!gProgram) {
    LOGE("Could not create program.");
    return false;
  }
  gvPositionHandle = glGetAttribLocation(gProgram, "vPosition");
  checkGlError("glGetAttribLocation");
  LOGI("glGetAttribLocation(\"vPosition\") = %d\n", gvPositionHandle);

  constexpr float kVertexData[18] = {
    -0.984375, -0.99609375,
    -0.99999, -0.98828125,
    -0.96875, -0.98828125,

    -0.96875, -0.984375,
    1., -1.,
    1., 1.,

    -0.96875, -0.984375,
    -1., 1.,
    1., 1.,
  };

  glGenBuffers(1, &gVertexBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, gVertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(kVertexData), kVertexData, GL_STATIC_DRAW);

  glUseProgram(gProgram);
  glVertexAttribPointer(gvPositionHandle, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
  glEnableVertexAttribArray(gvPositionHandle);

  // Set up color attachments and a framebuffer
  glGenTextures(kImageCount, gImages);
  glGenFramebuffers(1, &gFramebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, gFramebuffer);
  GLenum enabledBufs[kImageCount];
  for (uint32_t i = 0; i < kImageCount; ++i) {
    glBindTexture(GL_TEXTURE_2D, gImages[i]);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, kImageWidth, kImageHeight);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, gImages[i], 0);
    enabledBufs[i] = GL_COLOR_ATTACHMENT0 + i;
  }
  glDrawBuffers(kImageCount, enabledBufs);

  glViewport(0, 0, kImageWidth, kImageHeight);
  checkGlError("glViewport");
  return true;
}

void renderFrame() {
  static float grey;
  grey += 0.01f;
  if (grey > 1.0f) {
    grey = 0.0f;
  }

  glBindFramebuffer(GL_FRAMEBUFFER, gFramebuffer);

  // Draw.  Clear before the render pass to use the equivalent of Vulkan's
  // VK_ATTACHMENT_LOAD_OP_CLEAR (and avoid VK_ATTACHMENT_LOAD_OP_LOAD).
  glClearColor(grey, grey, grey, 1.0f);
  checkGlError("glClearColor");
  glClear(GL_COLOR_BUFFER_BIT);
  checkGlError("glClear");

  glUseProgram(gProgram);
  checkGlError("glUseProgram");

  glDrawArrays(GL_TRIANGLES, 0, 9);
  checkGlError("glDrawArrays");

  // Blit to the default framebuffer for verification.
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  glBlitFramebuffer(0, 0, 16, 16, 0, 0, gWindowWidth, gWindowHeight,
      GL_COLOR_BUFFER_BIT, GL_NEAREST);
  checkGlError("glBlitFramebuffer");
}

extern "C" {
JNIEXPORT void JNICALL Java_com_android_gl2jni_GL2JNILib_init(JNIEnv* env,
                                                              jobject obj,
                                                              jint width,
                                                              jint height);
JNIEXPORT void JNICALL Java_com_android_gl2jni_GL2JNILib_step(JNIEnv* env,
                                                              jobject obj);
};

JNIEXPORT void JNICALL Java_com_android_gl2jni_GL2JNILib_init(JNIEnv* env,
                                                              jobject obj,
                                                              jint width,
                                                              jint height) {
  gl3stubInit();
  setupGraphics(width, height);
}

JNIEXPORT void JNICALL Java_com_android_gl2jni_GL2JNILib_step(JNIEnv* env,
                                                              jobject obj) {
  renderFrame();
}
