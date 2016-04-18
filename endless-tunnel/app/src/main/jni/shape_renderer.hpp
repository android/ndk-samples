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
#ifndef endlesstunnel_shape_renderer_hpp
#define endlesstunnel_shape_renderer_hpp

#include "engine.hpp"

/* Convenience class that renders shapes (currently, only rects). The
 * coordinate system is the "normalized 2D coordinate system" -- see
 * README for more info. */
class ShapeRenderer {
    private:
        TrivialShader *mTrivialShader;
        float mColor[3];
        SimpleGeom* mGeom;

    public:
        ShapeRenderer(TrivialShader *trivialShader);
        ~ShapeRenderer();

        void SetColor(float r, float g, float b) {
            mColor[0] = r, mColor[1] = g, mColor[2] = b;
        }
        void SetColor(const float *v) {
            mColor[0] = v[0], mColor[1] = v[1], mColor[2] = v[2];
        }

        // Render a rectangle
        void RenderRect(float centerX, float centerY, float width, float height);
};

#endif

