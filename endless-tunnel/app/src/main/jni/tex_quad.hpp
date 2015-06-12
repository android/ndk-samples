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
#ifndef endlesstunnel_texquad_hpp
#define endlesstunnel_texquad_hpp

#include "engine.hpp"
#include "our_shader.hpp"

// Represents a simple 2D textured quad (that can be used to render an icon, for example)
class TexQuad {
    private:
        Texture *mTexture;
        OurShader *mOurShader;
        SimpleGeom *mGeom;
        float mWidth, mHeight;
        float mScale;
        float mCenterX, mCenterY;
        float mAspect;
        void CreateGeom(float umin, float vmin, float umax, float vmax);
        inline void Init(Texture *t, OurShader *shader, float aspect,
                float umin, float vmin, float umax, float vmax) {
            mTexture = t;
            mOurShader = shader;
            mAspect = aspect;
            mWidth = mAspect;
            mHeight = 1.0f;
            mCenterX = mCenterY = 0.0f;
            mScale = 1.0f;
            CreateGeom(umin, vmin, umax, vmax);
        }
    public:
        inline TexQuad(Texture *t, OurShader *shader, float aspect, float umin, float vmin,
                float umax, float vmax) {
            Init(t, shader, aspect, umin, vmin, umax, vmax);
        }
        inline TexQuad(Texture *t, OurShader *shader, float umin, float vmin, float umax,
               float vmax) {
            Init(t, shader, (umax - umin) / (vmax - vmin), umin, vmin, umax, vmax);
        }
        inline float GetCenterX() { return mCenterX; }
        inline float GetCenterY() { return mCenterY; }
        inline float GetWidth() { return mWidth; }
        inline float GetHeight() { return mHeight; }
        inline float GetLeft() { return mCenterX - mWidth * 0.5f; }
        inline float GetRight() { return mCenterX + mWidth * 0.5f; }
        inline float GetBottom() { return mCenterY - mHeight * 0.5f; }
        inline float GetTop() { return mCenterY + mHeight * 0.5f; }
        inline bool Contains(float x, float y) {
            return x >= GetLeft() && x <= GetRight() && y >= GetBottom() && y <= GetTop();
        }

        void SetCenter(float x, float y) {
            mCenterX = x;
            mCenterY = y;
        }
        void SetWidth(float w) {
            mWidth = w;
            mHeight = w / mAspect;
        }
        void SetHeight(float h) {
            mHeight = h;
            mWidth = h * mAspect;
        }
        inline ~TexQuad() {
            if (mGeom) {
                delete mGeom;
            }
        }
        inline void SetScale(float scale) {
            mScale = scale;
        }
        inline void Render() {
            Render(NULL);
        }
        void Render(glm::mat4 *transform);
};

#endif

