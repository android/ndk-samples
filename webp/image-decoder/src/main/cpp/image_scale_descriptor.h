/*
 * Copyright (C) 2021 The Android Open Source Project
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

#include <android/rect.h>
#include <cstring>

class ImageScaleDescriptor {
  public:
    explicit ImageScaleDescriptor(ARect dRect, ARect sRect) {
        requestRect = dRect;
        srcRect = sRect;
        memset(&resultRect, 0, sizeof(requestRect));
        rotationAngle = 0;
    };

    /**
     * Calculate the min scaled rectangle size.
     * @param rect: resulting scaled rectangle
     * @return counter clockwise rotation angle
     */
    int32_t getScaleRectInfo(ARect* _Nonnull rect) {
        bool srcPortrait = true;
        int32_t srcH = srcRect.bottom - srcRect.top;
        int32_t srcW = srcRect.right - srcRect.left;
        if(srcW > srcH) srcPortrait = false;

        bool dstPortrait = true;
        int32_t dstH = requestRect.bottom - requestRect.top;
        int32_t dstW = requestRect.right - requestRect.left;
        if(dstW > dstH) dstPortrait = false;

        int32_t height =0, width = 0;
        if(dstPortrait != srcPortrait) {
            rotationAngle = 270;
            float hRatio = static_cast<float>(dstH) / srcW;
            float wRatio = static_cast<float>(dstW) / srcH;

            float ratio = (hRatio < wRatio)? hRatio : wRatio;

            width = static_cast<int32_t>(ratio * srcW);
            height = static_cast<int32_t>(ratio * srcH);

        } else {
            rotationAngle = 0;
            float hRatio = static_cast<float>(dstH) / srcH;
            float wRatio = static_cast<float>(dstW) / srcW;
            float ratio = (hRatio < wRatio) ? hRatio : wRatio;
            height = static_cast<int32_t>(ratio * srcH);
            width = static_cast<int32_t>(ratio * srcW);
        }

        rect->left = 0;
        rect->right = width;
        rect->top = 0;
        rect->bottom = height;
        return rotationAngle;
    }

    ~ImageScaleDescriptor() {};

private:
    ARect requestRect, srcRect, resultRect;
    int32_t rotationAngle;  // Counter Clock rotation Angle
};


