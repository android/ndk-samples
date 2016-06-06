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
#include "vertexbuf.hpp"

VertexBuf::VertexBuf(GLfloat *geomData, int dataSize, int stride) {
    MY_ASSERT(dataSize % stride == 0);

    mPrimitive = GL_TRIANGLES;
    mVbo = 0;
    mStride = stride;
    mColorsOffset = mTexCoordsOffset = 0;
    mCount = dataSize / stride;

    // build VBO
    glGenBuffers(1, &mVbo);
    BindBuffer();
    glBufferData(GL_ARRAY_BUFFER, dataSize, geomData, GL_STATIC_DRAW);
    UnbindBuffer();
}

void VertexBuf::BindBuffer() {
    glBindBuffer(GL_ARRAY_BUFFER, mVbo);
}

void VertexBuf::UnbindBuffer() {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

VertexBuf::~VertexBuf() {
   glDeleteBuffers(1, &mVbo);
   mVbo = 0;
}


