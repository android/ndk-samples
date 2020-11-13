//
// Copyright (C) 2018 The Android Open Source Project
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
//  ShaderPlain.fsh
//

#define USE_PHONG (1)

uniform lowp vec3       vMaterialAmbient;
uniform lowp vec4       vMaterialSpecular;

uniform sampler2D samplerObj;

varying mediump vec2    texCoord;
varying lowp vec4 diffuseLight;

#if USE_PHONG
uniform highp vec3   vLight0;
varying mediump vec3 position;
varying mediump vec3 normal;
#else
varying lowp vec4 colorSpecular;
#endif

void main()
{
    mediump vec2 flippedTexCoord = vec2(texCoord.x, -texCoord.y);
#if USE_PHONG
    mediump vec3 halfVector = normalize(-vLight0 + position);
    mediump float NdotH = max(dot(normalize(normal), halfVector), 0.0);
    mediump float fPower = vMaterialSpecular.w;
    mediump float specular = pow(NdotH, fPower);

    lowp vec4 colorSpecular = vec4( vMaterialSpecular.xyz * specular, 1 );

    // increase ambient light to brighten the teapot :-)
    gl_FragColor = diffuseLight * texture2D(samplerObj, flippedTexCoord) +
            2.0f * vec4(vMaterialAmbient.xyz, 1.0f) + colorSpecular;
#else
    gl_FragColor = mix(colorDiffuse, texture2D(samplerObj, flippedTexCoord), 0.5) + colorSpecular;
#endif
}
