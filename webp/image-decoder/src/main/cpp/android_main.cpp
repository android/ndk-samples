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

#include "image_viewer.h"
#include "android_debug.h"

static int32_t ProcessAndroidInput(struct android_app *app, AInputEvent *event) {
    ImageViewer* imageViewer = reinterpret_cast<ImageViewer*>(app->userData);
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        imageViewer->StartAnimation(true);
        return 1;
    } else if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_KEY) {
        LOGI("Key event: action=%d keyCode=%d metaState=0x%x",
             AKeyEvent_getAction(event),
             AKeyEvent_getKeyCode(event),
             AKeyEvent_getMetaState(event));
    }

    return 0;
}

static void ProcessAndroidCmd(struct android_app *app, int32_t cmd) {
    static int32_t format = WINDOW_FORMAT_RGBA_8888;
    ImageViewer* imageViewer = reinterpret_cast<ImageViewer*>(app->userData);
    switch (cmd) {
        case APP_CMD_INIT_WINDOW:
            if (imageViewer->GetAndroidApp()->window) {
                // save current format to format variable, and set
                // display format to 8888
                format = ANativeWindow_getFormat(app->window);
                ANativeWindow_setBuffersGeometry(app->window,
                                                 ANativeWindow_getWidth(app->window),
                                                 ANativeWindow_getHeight(app->window),
                                                 WINDOW_FORMAT_RGBA_8888);
                imageViewer->PrepareDrawing();
                imageViewer->UpdateDisplay();
                imageViewer->StartAnimation(true);
            }
            break;
        case APP_CMD_TERM_WINDOW:
            imageViewer->StartAnimation(false);
            imageViewer->TerminateDisplay();
            ANativeWindow_setBuffersGeometry(app->window,
                                             ANativeWindow_getWidth(app->window),
                                             ANativeWindow_getHeight(app->window),
                                             format);
            break;
        case APP_CMD_LOST_FOCUS:
            imageViewer->StartAnimation(false);
            imageViewer->UpdateDisplay();
            break;
    }
}

// Android application glue entry function for us
extern "C" void android_main(struct android_app* state) {

    ImageViewer imageViewer(state);

    state->userData = reinterpret_cast<void*>(&imageViewer);
    state->onAppCmd = ProcessAndroidCmd;
    state->onInputEvent = ProcessAndroidInput;

    // loop waiting for stuff to do.
    while (1) {
        // Read all pending events.
        int ident;
        int events;
        struct android_poll_source* source;

        // If not animating, we will block forever waiting for events.
        // If animating, we loop until all events are read, then continue
        // to draw the next frame of animation.
        while ((ident = ALooper_pollAll(imageViewer.IsAnimating() ? 0 : -1, NULL, &events,
                                        (void**)&source)) >= 0) {

            // Process this event.
            if (source != NULL) {
                source->process(state, source);
            }

            // Check if we are exiting.
            if (state->destroyRequested != 0) {
                LOGI("Engine thread destroy requested!");
                imageViewer.TerminateDisplay();
                return;
            }
        }

        if (imageViewer.IsAnimating()) {
            imageViewer.UpdateDisplay();
        }
    }
}
