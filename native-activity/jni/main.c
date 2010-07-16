/*
 * Copyright (C) 2010 The Android Open Source Project
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
 *
 */

#include <jni.h>

#include <errno.h>

#include <android_native_app_glue.h>

#include "glutils.h"

struct engine {
    struct android_app* app;

    int animating;
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    int32_t width;
    int32_t height;
    float angle;
    int32_t x;
    int32_t y;
};

static int engine_init_display(struct engine* engine) {
    // initialize opengl and egl
    const EGLint attribs[] = {
            EGL_DEPTH_SIZE, 16,
            EGL_NONE
    };
    EGLint w, h, dummy;
    EGLint numConfigs;
    EGLConfig config;
    EGLSurface surface;
    EGLContext context;

    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    eglInitialize(display, 0, 0);
    selectConfigForNativeWindow(display, attribs, engine->app->window, &config);
    surface = eglCreateWindowSurface(display, config, engine->app->window, NULL);
    context = eglCreateContext(display, config, NULL, NULL);
    eglQuerySurface(display, surface, EGL_WIDTH, &w);
    eglQuerySurface(display, surface, EGL_HEIGHT, &h);

    if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
        LOGW("Unable to eglMakeCurrent");
        return -1;
    }

    engine->display = display;
    engine->context = context;
    engine->surface = surface;
    engine->width = w;
    engine->height = h;
    engine->angle = 0;

    // Initialize GL state.
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
    glEnable(GL_CULL_FACE);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);

    return 0;
}

static void engine_draw_frame(struct engine* engine) {
    if (engine->display == NULL) {
        // No display.
        return;
    }

    glClearColor(((float)engine->x)/engine->width, engine->angle,
            ((float)engine->y)/engine->height, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#if 0
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0, 0, -3.0f);
    glRotatef(engine->angle,        0, 1, 0);
    glRotatef(engine->angle*0.25f,  1, 0, 0);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    //mCube.draw(gl);

    glRotatef(engine->angle*2.0f, 0, 1, 1);
    glTranslatef(0.5f, 0.5f, 0.5f);

    //mCube.draw(gl);
#endif

    eglSwapBuffers(engine->display, engine->surface);

    //engine->angle += 1.2f;
}

static int engine_term_display(struct engine* engine) {
    if (engine->display != EGL_NO_DISPLAY) {
        eglMakeCurrent(engine->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (engine->context != EGL_NO_CONTEXT) {
            eglDestroyContext(engine->display, engine->context);
        }
        if (engine->surface != EGL_NO_SURFACE) {
            eglDestroySurface(engine->display, engine->surface);
        }
        eglTerminate(engine->display);
    }
    engine->animating = 0;
    engine->display = EGL_NO_DISPLAY;
    engine->context = EGL_NO_CONTEXT;
    engine->surface = EGL_NO_SURFACE;
}

static int engine_do_ui_event(struct engine* engine) {
    AInputEvent* event = NULL;
    if (AInputQueue_getEvent(engine->app->inputQueue, &event) >= 0) {
        LOGI("New input event: type=%d\n", AInputEvent_getType(event));
        if (AInputQueue_preDispatchEvent(engine->app->inputQueue, event)) {
            return 1;
        }
        if (AInputEvent_getType(event) == INPUT_EVENT_TYPE_MOTION) {
            engine->animating = 1;
            engine->x = AMotionEvent_getX(event, 0);
            engine->y = AMotionEvent_getY(event, 0);
            AInputQueue_finishEvent(engine->app->inputQueue, event, 1);
        } else {
            AInputQueue_finishEvent(engine->app->inputQueue, event, 0);
        }
    } else {
        LOGI("Failure reading next input event: %s\n", strerror(errno));
    }

    return 1;
}

static int32_t engine_do_main_cmd(struct engine* engine) {
    int32_t res;
    int8_t cmd = android_app_read_cmd(engine->app);
    switch (cmd) {
        case APP_CMD_WINDOW_CHANGED:
            engine_term_display(engine);
            res = android_app_exec_cmd(engine->app, cmd);
            if (engine->app->window != NULL) {
                engine_init_display(engine);
                engine_draw_frame(engine);
            }
            break;
        case APP_CMD_LOST_FOCUS:
            res = android_app_exec_cmd(engine->app, cmd);
            engine->animating = 0;
            engine_draw_frame(engine);
            break;
        default:
            res = android_app_exec_cmd(engine->app, cmd);
            break;
    }

    return res;
}

void android_main(struct android_app* state) {
    struct engine engine;

    memset(&engine, 0, sizeof(engine));
    state->userData = &engine;
    engine.app = state;

    // loop waiting for stuff to do.

    while (1) {
        // Read all pending events.
        int fd;
        int events;
        void* data;
        while ((fd=ALooper_pollAll(engine.animating ? 0 : -1, &events, &data)) >= 0) {
            switch ((int)data) {
                case LOOPER_ID_MAIN:
                    if (!engine_do_main_cmd(&engine)) {
                        LOGI("Engine thread destroy requested!");
                        engine_term_display(&engine);
                        return;
                    }
                    break;
                case LOOPER_ID_EVENT:
                    engine_do_ui_event(&engine);
                    break;
            }
        }

        if (engine.animating) {
            // Done with events; draw next animation frame.
            engine.angle += .01f;
            if (engine.angle > 1) {
                engine.angle = 0;
            }
            engine_draw_frame(&engine);
        }
    }
}
