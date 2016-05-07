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
#ifndef _mygame_tunnel_geom_hpp
#define _mygame_tunnel_geom_hpp

#include "engine.hpp"
#include "game_consts.hpp"

// shorthand to make it simpler to hand-code the geometry below:
// (first char is the coord, second is 'B'ottom, 'R'ight, 'L'eft or 'T'op,
// 'N'ear or 'F'ar)
// and third is whether this accounts for the 'S'pacing or 'N'ot.
#define ZB -TUNNEL_HALF_H
#define ZT TUNNEL_HALF_H
#define XL -TUNNEL_HALF_W
#define XR TUNNEL_HALF_W
#define YN (TUNNEL_SECTION_LENGTH * -0.5f)
#define YF (TUNNEL_SECTION_LENGTH * 0.5f)
#define TCOLOR_M 0.10f,0.10f,0.10f,1.0f
#define TCOLOR_D 0.07f,0.07f,0.07f,1.0f

// max texture coord along X and Z axis (i.e. repetitions)
#define REPN 10.0f

// max texture coord along Y axis (i.e. repetitions)
#define REPF (REPN * TUNNEL_SECTION_LENGTH / (2 * TUNNEL_HALF_W))

// note: the color data is temporary (will be replaced with texture once texture is implemented)
static GLfloat TUNNEL_GEOM[] = {
    /* vert  0: x,y,z:*/ XL, YN, ZB,  /*tex u,v*/ 0.0f, 0.0f, /*color:*/ TCOLOR_M,
    /* vert  1: x,y,z:*/ XR, YN, ZB,  /*tex u,v*/ REPN, 0.0f, /*color:*/ TCOLOR_M,
    /* vert  2: x,y,z:*/ XR, YF, ZB,  /*tex u,v*/ REPN, REPF, /*color:*/ TCOLOR_M,
    /* vert  3: x,y,z:*/ XL, YF, ZB,  /*tex u,v*/ 0.0f, REPF, /*color:*/ TCOLOR_M,
    /* vert  4: x,y,z:*/ XL, YN, ZB,  /*tex u,v*/ 0.0f, 0.0f, /*color:*/ TCOLOR_D,
    /* vert  5: x,y,z:*/ XL, YF, ZB,  /*tex u,v*/ REPF, 0.0f, /*color:*/ TCOLOR_D,
    /* vert  6: x,y,z:*/ XL, YF, ZT,  /*tex u,v*/ REPF, REPN, /*color:*/ TCOLOR_D,
    /* vert  7: x,y,z:*/ XL, YN, ZT,  /*tex u,v*/ 0.0f, REPN, /*color:*/ TCOLOR_D,
    /* vert  8: x,y,z:*/ XR, YN, ZB,  /*tex u,v*/ REPF, 0.0f, /*color:*/ TCOLOR_D,
    /* vert  9: x,y,z:*/ XR, YN, ZT,  /*tex u,v*/ REPF, REPN, /*color:*/ TCOLOR_D,
    /* vert 10: x,y,z:*/ XR, YF, ZT,  /*tex u,v*/ 0.0f, REPN, /*color:*/ TCOLOR_D,
    /* vert 11: x,y,z:*/ XR, YF, ZB,  /*tex u,v*/ 0.0f, 0.0f, /*color:*/ TCOLOR_D,
    /* vert 12: x,y,z:*/ XL, YF, ZT,  /*tex u,v*/ 0.0f, 0.0f, /*color:*/ TCOLOR_M,
    /* vert 13: x,y,z:*/ XR, YF, ZT,  /*tex u,v*/ REPN, 0.0f, /*color:*/ TCOLOR_M,
    /* vert 14: x,y,z:*/ XR, YN, ZT,  /*tex u,v*/ REPN, REPF, /*color:*/ TCOLOR_M,
    /* vert 15: x,y,z:*/ XL, YN, ZT,  /*tex u,v*/ 0.0f, REPF, /*color:*/ TCOLOR_M,
};

static GLushort TUNNEL_GEOM_INDICES[] = {
     0,  1,  2,  0,  2,  3, // floor
     8,  9, 11, 11,  9, 10, // right wall
     4,  5,  7,  7,  5,  6, // left wall
    12, 13, 15, 15, 13, 14  // ceiling
};

#define TUNNEL_GEOM_STRIDE (9 * sizeof(GLfloat))
#define TUNNEL_GEOM_TEXCOORD_OFFSET (3 * sizeof(GLfloat))
#define TUNNEL_GEOM_COLOR_OFFSET (5 * sizeof(GLfloat))

#endif
