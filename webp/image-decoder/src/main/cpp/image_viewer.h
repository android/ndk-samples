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

#pragma once

#include <android/native_window.h>
#include <android_native_app_glue.h>
#include "image_decoder.h"
#include "display_timer.h"

/*
 * Main object handles Android window frame update, and use webp to decode
 * pictures
 */
class ImageViewer {
  public:
    explicit ImageViewer(android_app* _Nullable app);
    ~ImageViewer();

    // Functions to connect to native_activity
    struct android_app* _Nullable GetAndroidApp(void) const;
    void StartAnimation(bool start);
    bool IsAnimating(void) const;
    void TerminateDisplay(void);

     // PrepareDrawing(): Initialize the Engine with current native window geometry
     //   and blank current screen to avoid garbage displaying on device
    bool PrepareDrawing(void);

    // Update the next image into display window when
    //    - decoding is completed, and
    //    - the current image has been displayed longer than the requested time
    bool UpdateDisplay(void);

  private:
    void ClearFrameBuffer(ANativeWindow_Buffer* _Nonnull buf);
    void UpdateFrameBuffer(ANativeWindow_Buffer * _Nonnull buf,
            DecodeFrameDescriptor* _Nonnull frame);
    int32_t getNativeWindowBpp(ANativeWindow_Buffer* _Nonnull buf);

    struct android_app* _Nullable androidApp;
    ImageDecoder* _Nullable imgDecoder;
    bool animatingInProgress;
    DisplayTimer displayTimer;
};

