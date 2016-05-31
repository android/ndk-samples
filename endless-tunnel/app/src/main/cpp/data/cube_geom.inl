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
#ifndef _mygame_obstacle_geom_hpp
#define _mygame_obstacle_geom_hpp

#include "engine.hpp"

/*
    H+-------+G
    /.      /|
 D / .    C/ |
  +-------+..+F
  | /E    | /
  |/      |/
 A+-------+B

*/
#define V_A  -0.5f, -0.5f,  0.5f
#define V_B   0.5f, -0.5f,  0.5f
#define V_C   0.5f,  0.5f,  0.5f
#define V_D  -0.5f,  0.5f,  0.5f
#define V_E  -0.5f, -0.5f, -0.5f
#define V_F   0.5f, -0.5f, -0.5f
#define V_G   0.5f,  0.5f, -0.5f
#define V_H  -0.5f,  0.5f, -0.5f
#define COLOR_1  1.0f, 1.0f, 1.0f, 1.0f
#define COLOR_2  0.8f, 0.8f, 0.8f, 1.0f
#define COLOR_3  0.6f, 0.6f, 0.6f, 1.0f

// max tex coordinate (i.e. texture repeats across cube)
#define TC_R 3.0f

static GLfloat CUBE_GEOM[] = {
   // front face of cube:
   V_A, COLOR_2, 0.0f, 0.0f,
   V_B, COLOR_2, TC_R, 0.0f,
   V_D, COLOR_2, 0.0f, TC_R,
   V_D, COLOR_2, 0.0f, TC_R,
   V_B, COLOR_2, TC_R, 0.0f,
   V_C, COLOR_2, TC_R, TC_R,
   // right face of cube:
   V_B, COLOR_3, 0.0f, 0.0f,
   V_F, COLOR_3, TC_R, 0.0f,
   V_C, COLOR_3, 0.0f, TC_R,
   V_C, COLOR_3, 0.0f, TC_R,
   V_F, COLOR_3, TC_R, 0.0f,
   V_G, COLOR_3, TC_R, TC_R,
   // left face of cube
   V_A, COLOR_3, 0.0f, TC_R,
   V_D, COLOR_3, TC_R, TC_R,
   V_E, COLOR_3, 0.0f, 0.0f,
   V_E, COLOR_3, 0.0f, 0.0f,
   V_D, COLOR_3, TC_R, TC_R,
   V_H, COLOR_3, 0.0f, TC_R,
   // back face of cube
   V_E, COLOR_2, TC_R, 0.0f,
   V_H, COLOR_2, TC_R, TC_R,
   V_F, COLOR_2, 0.0f, 0.0f,
   V_F, COLOR_2, 0.0f, 0.0f,
   V_H, COLOR_2, TC_R, TC_R,
   V_G, COLOR_2, 0.0f, TC_R,
   // bottom of cube
   V_A, COLOR_1, 0.0f, TC_R,
   V_E, COLOR_1, 0.0f, 0.0f,
   V_B, COLOR_1, TC_R, TC_R,
   V_B, COLOR_1, TC_R, TC_R,
   V_E, COLOR_1, 0.0f, 0.0f,
   V_F, COLOR_1, TC_R, 0.0f,
   // top of cube
   V_D, COLOR_1, 0.0f, 0.0f,
   V_C, COLOR_1, TC_R, 0.0f,
   V_H, COLOR_1, 0.0f, TC_R,
   V_H, COLOR_1, 0.0f, TC_R,
   V_C, COLOR_1, TC_R, 0.0f,
   V_G, COLOR_1, TC_R, TC_R
};
static const int CUBE_GEOM_COLOR_OFFSET = 3 * sizeof(GLfloat);
static const int CUBE_GEOM_TEXCOORD_OFFSET = 7 * sizeof(GLfloat);
static const int CUBE_GEOM_STRIDE = 9 * sizeof(GLfloat);

/* this is something of a trivial index buffer (and could just as well be
   replaced by an array), but we have it here for demonstration purposes:
static unsigned short CUBE_GEOM_INDICES[] = {
    0, 1, 2, 3, 4, 5,
    6, 7, 8, 9, 10, 11,
    12, 13, 14, 15, 16, 17,
    18, 19, 20, 21, 22, 23,
    24, 25, 26, 27, 28, 29,
    30, 31, 32, 33, 34, 35
};
*/
#endif
