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
#ifndef endlesstunnel_shader_hpp
#define endlesstunnel_shader_hpp

#include "glm/glm.hpp"
#include "simplegeom.hpp"

class VertexBuf;
class IndexBuf;

/* Represents an OpenGL shader. This class is not meant to be used directly, but, rather
 * to be subclassed to represent specific shaders. To use any shader that's a subclass
 * of this class, first construct it, then call Compile(). After that, you can render
 * geometry by calling BeginRender(), then calling Render() as many times as you want,
 * and then EndRender(). This allows you to render the same geometry in multiple
 * places efficiently. If you just want to render a geometry once (simple use case),
 * you can call RenderSimpleGeom(). */
class Shader {
    protected:
        // OpenGL handles
        int mVertShaderH, mFragShaderH;
        GLuint mProgramH;
        int mMVPMatrixLoc;
        int mPositionAttribLoc;

        // Geometry we are rendering (this is only valid between BeginRender and EndRender)
        VertexBuf *mPreparedVertexBuf;
    public:
        Shader();
        virtual ~Shader();

        // compile shader
        virtual void Compile();

        // rendering:
        void BindShader();
        void UnbindShader();

        // Prepares to render the given geometry.
        virtual void BeginRender(VertexBuf *vbuf);

        // Renders one copy of the prepared geometry, given a model-view-projection matrix.
        void Render(glm::mat4 *mvpMat) {
            Render(NULL, mvpMat);
        }

        // Renders a subset (given by the index buffer) of the prepared geometry, using
        // the given model-view-projection matrix.
        virtual void Render(IndexBuf *ibuf, glm::mat4* mvpMat);

        // Finishes rendering (call this after you're done making calls to Render())
        virtual void EndRender();

        // Convenience method to render a single copy of a geometry.
        void RenderSimpleGeom(glm::mat4* mvpMat, SimpleGeom *sg) {
            BeginRender(sg->vbuf);
            Render(sg->ibuf, mvpMat);
            EndRender();
        }
    protected:
        // Push MVP matrix to the shader
        void PushMVPMatrix(glm::mat4 *mat);

        // Push the vertex positions to the shader
        void PushPositions(int vbo_offset, int stride);

        // Must return the vertex shader's GLSL source
        virtual const char* GetVertShaderSource() = 0;

        // Must return the fragment shader's GLSL source
        virtual const char* GetFragShaderSource() = 0;

        // Must return the shader's name (used for debug/logging purposes)
        virtual const char* GetShaderName() = 0;
};


/* A trivial shader that knows how to render geometry and colors, but no lighting
 * or texturing. Compatible with geometry that contains color data. You can also specify
 * a tint color, which will get multiplied by the geometry's built-in color. */
class TrivialShader : public Shader {
    protected:
        int mColorLoc;
        int mTintLoc;
        float mTint[3];
    public:
        TrivialShader();
        ~TrivialShader();
        int GetColorAttribLoc();
        void PushColors(int vbo_offset, int stride);
        void SetTintColor(float r, float g, float b);
        void ResetTintColor();
        virtual void Compile();
        virtual void BeginRender(VertexBuf *geom);
    protected:
        virtual const char* GetVertShaderSource();
        virtual const char* GetFragShaderSource();
        virtual const char* GetShaderName();
};

#endif

