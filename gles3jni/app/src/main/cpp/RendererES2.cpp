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

#include "gles3jni.h"
#include <EGL/egl.h>

static const char VERTEX_SHADER[] =
    "#version 100\n"
    "uniform mat2 scaleRot;\n"
    "uniform vec2 offset;\n"
    "attribute vec2 pos;\n"
    "attribute vec4 color;\n"
    "varying vec4 vColor;\n"
    "void main() {\n"
    "    gl_Position = vec4(scaleRot*pos + offset, 0.0, 1.0);\n"
    "    vColor = color;\n"
    "}\n";

static const char FRAGMENT_SHADER[] =
    "#version 100\n"
    "precision mediump float;\n"
    "varying vec4 vColor;\n"
    "void main() {\n"
    "    gl_FragColor = vColor;\n"
    "}\n";

class RendererES2: public Renderer {
public:
    RendererES2();
    virtual ~RendererES2();
    bool init();

private:
    virtual float* mapOffsetBuf();
    virtual void unmapOffsetBuf();
    virtual float* mapTransformBuf();
    virtual void unmapTransformBuf();
    virtual void draw(unsigned int numInstances);

    const EGLContext mEglContext;
    GLuint mProgram;
    GLuint mVB;
    GLint mPosAttrib;
    GLint mColorAttrib;
    GLint mScaleRotUniform;
    GLint mOffsetUniform;

    float mOffsets[2*MAX_INSTANCES];
    float mScaleRot[4*MAX_INSTANCES];   // array of 2x2 column-major matrices
};

Renderer* createES2Renderer() {
    RendererES2* renderer = new RendererES2;
    if (!renderer->init()) {
        delete renderer;
        return NULL;
    }
    return renderer;
}

RendererES2::RendererES2()
:   mEglContext(eglGetCurrentContext()),
    mProgram(0),
    mVB(0),
    mPosAttrib(-1),
    mColorAttrib(-1),
    mScaleRotUniform(-1),
    mOffsetUniform(-1)
{}

bool RendererES2::init() {
    mProgram = createProgram(VERTEX_SHADER, FRAGMENT_SHADER);
    if (!mProgram)
        return false;
    mPosAttrib = glGetAttribLocation(mProgram, "pos");
    mColorAttrib = glGetAttribLocation(mProgram, "color");
    mScaleRotUniform = glGetUniformLocation(mProgram, "scaleRot");
    mOffsetUniform = glGetUniformLocation(mProgram, "offset");

    glGenBuffers(1, &mVB);
    glBindBuffer(GL_ARRAY_BUFFER, mVB);
    glBufferData(GL_ARRAY_BUFFER, sizeof(QUAD), &QUAD[0], GL_STATIC_DRAW);

    ALOGV("Using OpenGL ES 2.0 renderer");
    return true;
}

RendererES2::~RendererES2() {
    /* The destructor may be called after the context has already been
     * destroyed, in which case our objects have already been destroyed.
     *
     * If the context exists, it must be current. This only happens when we're
     * cleaning up after a failed init().
     */
    if (eglGetCurrentContext() != mEglContext)
        return;
    glDeleteBuffers(1, &mVB);
    glDeleteProgram(mProgram);
}

float* RendererES2::mapOffsetBuf() {
    return mOffsets;
}

void RendererES2::unmapOffsetBuf() {
}

float* RendererES2::mapTransformBuf() {
    return mScaleRot;
}

void RendererES2::unmapTransformBuf() {
}

void RendererES2::draw(unsigned int numInstances) {
    glUseProgram(mProgram);

    glBindBuffer(GL_ARRAY_BUFFER, mVB);
    glVertexAttribPointer(mPosAttrib, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)offsetof(Vertex, pos));
    glVertexAttribPointer(mColorAttrib, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (const GLvoid*)offsetof(Vertex, rgba));
    glEnableVertexAttribArray(mPosAttrib);
    glEnableVertexAttribArray(mColorAttrib);

    for (unsigned int i = 0; i < numInstances; i++) {
        glUniformMatrix2fv(mScaleRotUniform, 1, GL_FALSE, mScaleRot + 4*i);
        glUniform2fv(mOffsetUniform, 1, mOffsets + 2*i);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
}
