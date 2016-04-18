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
#include "ascii_to_geom.hpp"

#define GEOM_DEBUG LOGD
//#define GEOM_DEBUG

SimpleGeom* AsciiArtToGeom(const char *art, float scale) {
    // figure out width and height
    LOGD("Creating geometry from ASCII art.");
    GEOM_DEBUG("Ascii art source:\n%s", art);
    int rows = 1;
    int curCols = 0, cols = 0;
    int r, c;
    const char *p;
    for (p = art; *p; ++p) {
        if (*p == '\n') {
            rows++;
            curCols = 0;
        } else {
            curCols++;
            cols = curCols > cols ? curCols : cols;
        }
    }

    GEOM_DEBUG("Ascii art has %d rows, %d cols.", rows, cols);
    GEOM_DEBUG("Making working array.");

    // allocate a rows x cols array that we will use as working space
    unsigned int **v = new unsigned int*[rows];
    for (r = 0; r < rows; r++) {
        v[r] = new unsigned int[cols];
        memset(v[r], 0, cols * sizeof(unsigned int));
    }

    // copy the input into the array
    r = c = 0;
    for (p = art; *p; ++p) {
        if (*p == '\n') {
            r++, c=0;
        } else {
            MY_ASSERT(r >= 0 && r < rows);
            MY_ASSERT(c >= 0 && c < cols);
            v[r][c++] = static_cast<unsigned int>(*p);
        }
    }

    GEOM_DEBUG("Removing redundant line markers.");

    // remove redundant line markers
    for (r = 0; r < rows; r++) {
        for (c = 0; c < cols; c++) {
            if (c + 1 < cols && v[r][c] == '-' && v[r][c+1] == '-') {
                v[r][c] = ' ';
            }
            if (r + 1 < rows && v[r][c] == '|' && v[r+1][c] == '|') {
                v[r][c] = ' ';
            }
            if (r + 1 < rows && c + 1 < cols && v[r][c] == '`' && v[r+1][c+1] == '`') {
                v[r][c] = ' ';
            }
            if (r + 1 < rows && c > 0 && v[r][c] == '/' && v[r+1][c-1] == '/') {
                v[r][c] = ' ';
            }
        }
    }

    // count how many vertices and indices we will have
    int vertices = 0, indices = 0;
    for (r = 0; r < rows; r++) {
        for (c = 0; c < cols; c++) {
            char t = static_cast<char>(v[r][c]);
            if (t == '+') {
                vertices++;
            } else if (t == '-' || t == '|' || t == '`' || t == '/') {
                indices += 2; // each line requires 2 indices
            }
        }
    }

    GEOM_DEBUG("Total vertices: %d, total indices %d", vertices, indices);

    // allocate arrays for the vertices and lines
    const int VERTICES_STRIDE = sizeof(GLfloat) * 7;
    const int VERTICES_COLOR_OFFSET = sizeof(GLfloat) * 3;
    GLfloat *verticesArray = new GLfloat[vertices * VERTICES_STRIDE];
    GLushort *indicesArray = new GLushort[indices];
    vertices = indices = 0; // current count of vertices and lines

    float left = (-cols/2) * scale;
    if (cols % 2 == 0) left += scale * 0.5f;
    float top = (rows/2) * scale;
    if (rows % 2 == 0) top += scale * 0.5f;

    const int VERTEX_BIT = 0x1000;
    const int VERTEX_INDEX_MASK = 0x0fff;

    // process vertices
    for (r = 0; r < rows; r++) {
        for (c = 0; c < cols; c++) {
            unsigned t = v[r][c];
            if (t == '+') {
                GEOM_DEBUG("Found vertex at %d,%d, index %d", r, c, vertices);
                verticesArray[vertices * 7] = left + c * scale;
                verticesArray[vertices * 7 + 1] = top - r * scale;
                verticesArray[vertices * 7 + 2] = 0.0f; // z coord is always 0
                verticesArray[vertices * 7 + 3] = 1.0f; // red
                verticesArray[vertices * 7 + 4] = 1.0f; // green
                verticesArray[vertices * 7 + 5] = 1.0f; // blue
                verticesArray[vertices * 7 + 6] = 1.0f; // alpha
                // mark which vertex this is
                v[r][c] = static_cast<unsigned int>(VERTEX_BIT | vertices);
                vertices++;
            }
        }
    }

    // process lines
    int col_dir, row_dir;
    int start_c, start_r, end_c, end_r;

    GEOM_DEBUG("Now processing lines.");
    for (r = 0; r < rows; r++) {
        for (c = 0; c < cols; c++) {
            int t = v[r][c];
            if (t == '-') {
                // horizontal line
                GEOM_DEBUG("Horizontal line found at %d,%d", r,c);
                col_dir = -1, row_dir = 0;
            } else if (t == '|') {
                // vertical line
                GEOM_DEBUG("Vertical line found at %d,%d", r,c);
                col_dir = 0, row_dir = -1;
            } else if (t == '`') {
                // horizontal line, slanting down
                GEOM_DEBUG("Downward diagonal line found at %d,%d", r,c);
                col_dir = -1, row_dir = -1;
            } else if (t == '/') {
                // horizontal line, slanting down
                GEOM_DEBUG("Upward diagonal line found at %d,%d", r,c);
                col_dir = -1, row_dir = 1;
            } else {
                continue;
            }

            // look for the vertex that starts the line:
            start_c = c;
            start_r = r;
            while (!(v[start_r][start_c] & VERTEX_BIT)) {
                start_c += col_dir;
                start_r += row_dir;
                if (start_c < 0 || start_r < 0 || start_c >= cols || start_r >= rows) {
                    LOGE("Invalid line in ascii-art: no start. At position %d,%d", r, c);
                    ABORT_GAME;
                }
            }
            GEOM_DEBUG("Start vertex is at %d,%d, index %d", start_r, start_c,
                    v[start_r][start_c] & VERTEX_INDEX_MASK);

            // look for the vertex that ends the line
            end_c = c;
            end_r = r;
            while (!(v[end_r][end_c] & VERTEX_BIT)) {
                end_c -= col_dir;
                end_r -= row_dir;
                if (end_c < 0 || end_r < 0 || end_c >= cols || end_r >= rows) {
                    LOGE("Invalid line in ascii-art: no end. At position %d,%d", r, c);
                    ABORT_GAME;
                }
            }

            GEOM_DEBUG("End vertex is at %d,%d, index %d", end_r, end_c,
                    v[end_r][end_c] & VERTEX_INDEX_MASK);

            indicesArray[indices] = static_cast<GLushort>(v[start_r][start_c] & VERTEX_INDEX_MASK);
            indicesArray[indices + 1] = static_cast<GLushort>(v[end_r][end_c] & VERTEX_INDEX_MASK);
            indices += 2;
            GEOM_DEBUG("We now have %d indices.", indices);
        }
    }

    GEOM_DEBUG("Deallocating working space.");
    // get rid of the working arrays
    for (r = 0; r < rows; r++) {
        delete v[r];
    }
    delete [] v;

    for (int i = 0; i < indices; i++) {
        GEOM_DEBUG("indices[%d] = %d\n", i, indicesArray[i]);
    }
    for (int i = 0; i < vertices; i++) {
        GEOM_DEBUG("vertices[%d]", i*7);
        for (int j = 0; j < 7; j++) {
            GEOM_DEBUG("vertices[%d+%d=%d] = %f\n", i*7, j, i*7+j, verticesArray[i*7+j]);
        }
    }

    // create the buffers
    GEOM_DEBUG("Creating output VBO (%d vertices) and IBO (%d indices).", vertices, indices);
    SimpleGeom* out = new SimpleGeom(new VertexBuf(verticesArray, vertices * sizeof(GLfloat) *
            VERTICES_STRIDE, VERTICES_STRIDE), new IndexBuf(indicesArray, indices *
            sizeof(GLushort)));
    out->vbuf->SetPrimitive(GL_LINES);  // draw as lines
    out->vbuf->SetColorsOffset(VERTICES_COLOR_OFFSET);

    // clean up our work buffers
    delete [] verticesArray;
    verticesArray = NULL;
    delete [] indicesArray;
    indicesArray = NULL;

    LOGD("Created geometry from ascii art: %d vertices, %d indices", vertices, indices);

    return out;
}

