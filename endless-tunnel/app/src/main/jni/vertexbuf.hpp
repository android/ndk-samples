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
#ifndef endlesstunnel_vertexbuf_hpp
#define endlesstunnel_vertexbuf_hpp

#include "common.hpp"

/* Represents a vertex buffer (VBO). */
class VertexBuf {
    private:
        GLuint mVbo;
        GLenum mPrimitive;
        int mStride;
        int mColorsOffset;
        int mTexCoordsOffset;
        int mCount;

    public:
        VertexBuf(GLfloat *geomData, int dataSize, int stride);
        ~VertexBuf();

        void BindBuffer();
        void UnbindBuffer();

        int GetStride() { return mStride; }
        int GetCount() { return mCount; }
        int GetPositionsOffset() { return 0; }

        bool HasColors() { return mColorsOffset > 0; }
        int GetColorsOffset() { return mColorsOffset; }
        void SetColorsOffset(int offset) { mColorsOffset = offset; }

        bool HasTexCoords() { return mTexCoordsOffset > 0; }
        void SetTexCoordsOffset(int offset) { mTexCoordsOffset = offset; }
        int GetTexCoordsOffset() { return mTexCoordsOffset; }

        GLenum GetPrimitive() { return mPrimitive; }
        void SetPrimitive(GLenum primitive) { mPrimitive = primitive; }
};

#endif

