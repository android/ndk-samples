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
#ifndef _mygame_our_shader_inl
#define _mygame_our_shader_inl

#define OUR_VERTEX_SHADER_SOURCE \
           "uniform mat4 u_MVP;            \n" \
           "uniform vec4 u_PointLightPos;  \n" \
           "uniform mediump vec4 u_PointLightColor; \n" \
           "attribute vec4 a_Position;     \n" \
           "attribute vec4 a_Color;        \n" \
           "attribute vec2 a_TexCoord;     \n" \
           "varying vec4 v_Color;          \n" \
           "varying vec4 v_Pos;            \n" \
           "varying float v_FogFactor;     \n" \
           "varying vec2 v_TexCoord;      \n" \
           "float FOG_START = 100.0;        \n" \
           "float FOG_END = 200.0;         \n" \
           "varying vec4 v_PointLightPos;  \n" \
           "void main()                    \n" \
           "{                              \n" \
           "   v_Color = a_Color;          \n" \
           "   gl_Position = u_MVP         \n" \
           "               * a_Position;   \n" \
           "   v_Pos = u_MVP * a_Position; \n" \
           "   v_PointLightPos = u_MVP * u_PointLightPos; \n" \
           "   v_TexCoord = a_TexCoord;    \n" \
           "   v_FogFactor = clamp((v_Pos.z - FOG_START) / (FOG_END - FOG_START), 0.0, 1.0); \n" \
           "}                              \n";

#define OUR_FRAG_SHADER_SOURCE \
           "precision mediump float;       \n" \
           "varying vec4 v_Color;          \n" \
           "varying vec4 v_Pos;          \n" \
           "varying vec2 v_TexCoord;      \n" \
           "varying float v_FogFactor;     \n" \
           "uniform vec4 u_Tint;           \n" \
           "uniform sampler2D u_Sampler;   \n" \
           "uniform vec4 u_PointLightColor; \n" \
           "varying vec4 v_PointLightPos;   \n" \
           "float ATT_FACT_2 = 0.005;          \n" \
           "float ATT_FACT_1 = 0.00;          \n" \
           "void main()                    \n" \
           "{                              \n" \
           "   float d = distance(v_PointLightPos, v_Pos);\n" \
           "   float att = 1.0/(ATT_FACT_1 * d + ATT_FACT_2 * d * d);\n" \
           "   gl_FragColor = mix(v_Color * u_Tint * texture2D(u_Sampler, v_TexCoord) + u_PointLightColor * att, vec4(0), v_FogFactor);\n" \
           "}";

#endif

