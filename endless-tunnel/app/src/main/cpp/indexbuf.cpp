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
#include "indexbuf.hpp"

IndexBuf::IndexBuf(GLushort *data, int dataSizeBytes) {
    mCount = dataSizeBytes / sizeof(GLushort);

    glGenBuffers(1, &mIbo);
    BindBuffer();
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, dataSizeBytes, data, GL_STATIC_DRAW);
    UnbindBuffer();
}

IndexBuf::~IndexBuf() {
    glDeleteBuffers(1, &mIbo);
    mIbo = 0;
}

void IndexBuf::BindBuffer() {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIbo);
}

void IndexBuf::UnbindBuffer() {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

