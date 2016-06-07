//
// Copyright (C) 2015 The Android Open Source Project
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
#version 300 es
precision mediump float;

//
//Shader with phoneshading + geometry instancing support
//Parameters with %PARAM_NAME% will be replaced to actual parameter at compile time
//

const int NUM_OBJECTS = %NUM_TEAPOT%;
layout(location=%LOCATION_VERTEX%) in highp vec3    myVertex;
layout(location=%LOCATION_NORMAL%) in highp vec3    myNormal;

layout(std140) uniform ParamBlock {
    mat4      uPMatrix[NUM_OBJECTS];
    mat4      uMVMatrix[NUM_OBJECTS];
    vec3      vMaterialDiffuse[NUM_OBJECTS];
};

uniform highp vec3      vLight0;
uniform lowp vec3       vMaterialAmbient;
uniform lowp vec4       vMaterialSpecular;

out lowp    vec4    colorDiffuse;

out mediump vec3 position;
out mediump vec3 normal;

void main(void)
{
    highp vec4 p = vec4(myVertex,1);
    gl_Position = uPMatrix[gl_InstanceID%ARB%] * p;

    highp vec3 worldNormal = vec3(mat3(uMVMatrix[gl_InstanceID%ARB%][0].xyz,
            uMVMatrix[gl_InstanceID%ARB%][1].xyz,
            uMVMatrix[gl_InstanceID%ARB%][2].xyz) * myNormal);
    highp vec3 ecPosition = p.xyz;

    colorDiffuse = dot( worldNormal, normalize(-vLight0+ecPosition) ) * vec4(vMaterialDiffuse[gl_InstanceID%ARB%], 1.f)  + vec4( vMaterialAmbient, 1 );

    normal = worldNormal;
    position = ecPosition;
}
