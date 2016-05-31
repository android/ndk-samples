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
#include "tex_quad.hpp"

static void _put_vertex(float *v, float x, float y, float tex_u, float tex_v) {
    // position
    v[0] = x;
    v[1] = y;
    v[2] = 0.0f;

    // color
    v[3] = 1.0f;
    v[4] = 1.0f;
    v[5] = 1.0f;
    v[6] = 1.0f;

    // texture coords
    v[7] = tex_u;
    v[8] = tex_v;
}

void TexQuad::CreateGeom(float umin, float vmin, float umax, float vmax) {
    const int stride_floats = 9; // 3 for coords, 4 for color, 2 for tex coordinates
    const int stride_bytes = stride_floats * sizeof(GLfloat);
    int vertices = stride_floats * 4; // 4 vertices
    GLfloat *geom = new GLfloat[vertices];
    int geom_size = sizeof(GLfloat) * vertices;
    GLushort *indices = new GLushort[6]; // 6 indices
    int indices_size = sizeof(GLushort) * 6;
    float left = -mAspect * 0.5f;
    float right = mAspect * 0.5f;
    float bottom = -0.5f;
    float top = 0.5f;

    /*
      D+----------+C
       |          |
       |          |
      A+----------+B
    */

    _put_vertex(geom, left, bottom, umin, vmin); // point A
    _put_vertex(geom + stride_floats, right, bottom, umax, vmin); // point B
    _put_vertex(geom + 2 * stride_floats, right, top, umax, vmax); // point C
    _put_vertex(geom + 3 * stride_floats, left, top, umin, vmax); // point D

    indices[0] = 0;
    indices[1] = 1;
    indices[2] = 2;
    indices[3] = 0;
    indices[4] = 2;
    indices[5] = 3;

    // prepare geometry
    VertexBuf *vbuf = new VertexBuf(geom, geom_size, stride_bytes);
    vbuf->SetColorsOffset(3 * sizeof(GLfloat));
    vbuf->SetTexCoordsOffset(7 * sizeof(GLfloat));
    IndexBuf *ibuf = new IndexBuf(indices, indices_size);
    mGeom = new SimpleGeom(vbuf, ibuf);

    // clean up our temporary buffers
    delete [] geom;
    geom = NULL;
    delete [] indices;
    indices = NULL;
}

void TexQuad::Render(glm::mat4 *transform) {
    float aspect = SceneManager::GetInstance()->GetScreenAspect();
    glm::mat4 orthoMat = glm::ortho(0.0f, aspect, 0.0f, 1.0f);
    glm::mat4 modelMat, mat;

    bool hadDepthTest = glIsEnabled(GL_DEPTH_TEST);
    glDisable(GL_DEPTH_TEST);

    modelMat = glm::translate(glm::mat4(1.0f), glm::vec3(mCenterX, mCenterY, 0.0f));
    modelMat = glm::scale(modelMat, glm::vec3(mScale * mHeight, mScale * mHeight, 0.0f));
    if (transform) {
        mat = orthoMat * (*transform) * modelMat;
    } else {
        mat = orthoMat * modelMat;
    }

    mOurShader->BeginRender(mGeom->vbuf);
    mOurShader->SetTexture(mTexture);
    mOurShader->Render(mGeom->ibuf, &mat);
    mOurShader->EndRender();

    if (hadDepthTest) {
        glEnable(GL_DEPTH_TEST);
    }
}


