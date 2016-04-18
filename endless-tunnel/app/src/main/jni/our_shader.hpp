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
#ifndef endlesstunnel_our_shader_hpp
#define endlesstunnel_our_shader_hpp

#include "engine.hpp"

// An OpenGL shader that can apply a texture and a point light.
// We use to to render the tunnel and the obstacles.
class OurShader : public Shader {
    protected:
       GLint mColorLoc;
       GLint mTexCoordLoc;
       int mTintLoc;
       int mSamplerLoc;
       int mPointLightPosLoc;
       int mPointLightColorLoc;
    public:
       OurShader();
       virtual ~OurShader();
       virtual void Compile();
       void SetTexture(Texture *t);
       void SetTintColor(float r, float g, float b);
       void EnablePointLight(glm::vec3 pos, float r, float g, float b);
       void DisablePointLight();
       virtual void BeginRender(VertexBuf *geom);
   protected:
       virtual const char *GetVertShaderSource();
       virtual const char *GetFragShaderSource();
       virtual const char *GetShaderName();
};

#endif

