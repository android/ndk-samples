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
#include <poll.h>
#include <pthread.h>
#include <sched.h>
#include <string.h>
#include <sys/resource.h>

#include "glutils.h"

// --------------------------------------------------------------------
// Rendering and input engine thread
// --------------------------------------------------------------------

struct engine {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    
    int msgread;
    int msgwrite;
    
    ANativeActivity* activity;
    pthread_t thread;
    
    int running;
    int destroyed;
    ALooper* looper;
    AInputQueue* inputQueue;
    ANativeWindow* window;
    AInputQueue* pendingInputQueue;
    ANativeWindow* pendingWindow;
    
    // private to engine thread.
    int destroyRequested;
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

enum {
    ENGINE_CMD_INPUT_CHANGED,
    ENGINE_CMD_WINDOW_CHANGED,
    ENGINE_CMD_GAINED_FOCUS,
    ENGINE_CMD_LOST_FOCUS,
    ENGINE_CMD_DESTROY,
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
    selectConfigForNativeWindow(display, attribs, engine->window, &config);
    surface = eglCreateWindowSurface(display, config, engine->window, NULL);
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

static int engine_process_event(int fd, int events, void* param) {
    struct engine* engine = (struct engine*)param;
    
    AInputEvent* event = NULL;
    if (AInputQueue_getEvent(engine->inputQueue, &event) >= 0) {
        LOGI("New input event: type=%d\n", AInputEvent_getType(event));
        if (AInputEvent_getType(event) == INPUT_EVENT_TYPE_MOTION) {
            engine->animating = 1;
            engine->x = AMotionEvent_getX(event, 0);
            engine->y = AMotionEvent_getY(event, 0);
            AInputQueue_finishEvent(engine->inputQueue, event, 1);
        } else {
            AInputQueue_finishEvent(engine->inputQueue, event, 0);
        }
    } else {
        LOGI("Failure reading next input event: %s\n", strerror(errno));
    }
    
    return 1;
}

static int engine_process_cmd(int fd, int events, void* param) {
    struct engine* engine = (struct engine*)param;
    
    int8_t cmd;
    if (read(engine->msgread, &cmd, sizeof(cmd)) == sizeof(cmd)) {
        switch (cmd) {
            case ENGINE_CMD_INPUT_CHANGED:
                LOGI("Engine: ENGINE_CMD_INPUT_CHANGED\n");
                pthread_mutex_lock(&engine->mutex);
                if (engine->inputQueue != NULL) {
                    AInputQueue_detachLooper(engine->inputQueue);
                }
                engine->inputQueue = engine->pendingInputQueue;
                if (engine->inputQueue != NULL) {
                    LOGI("Attaching input queue to looper");
                    AInputQueue_attachLooper(engine->inputQueue,
                            engine->looper, engine_process_event, engine);
                }
                pthread_cond_broadcast(&engine->cond);
                pthread_mutex_unlock(&engine->mutex);
                break;

            case ENGINE_CMD_WINDOW_CHANGED:
                LOGI("Engine: ENGINE_CMD_WINDOW_CHANGED\n");
                engine_term_display(engine);
                pthread_mutex_lock(&engine->mutex);
                engine->window = engine->pendingWindow;
                pthread_cond_broadcast(&engine->cond);
                pthread_mutex_unlock(&engine->mutex);
                if (engine->window != NULL) {
                    engine_init_display(engine);
                    engine_draw_frame(engine);
                }
                break;

            case ENGINE_CMD_LOST_FOCUS:
                engine->animating = 0;
                engine_draw_frame(engine);
                break;

            case ENGINE_CMD_DESTROY:
                LOGI("Engine: ENGINE_CMD_DESTROY\n");
                engine->destroyRequested = 1;
                break;
        }
    } else {
        LOGW("No data on command pipe!");
    }
    
    return 1;
}

static void* engine_entry(void* param) {
    struct engine* engine = (struct engine*)param;
    
    ALooper* looper = ALooper_prepare();
    ALooper_setCallback(looper, engine->msgread, POLLIN, engine_process_cmd, engine);
    engine->looper = looper;
    
    pthread_mutex_lock(&engine->mutex);
    engine->running = 1;
    pthread_cond_broadcast(&engine->cond);
    pthread_mutex_unlock(&engine->mutex);
    
    // loop waiting for stuff to do.
    
    while (1) {
        // Read all pending events.
        while (ALooper_pollOnce(engine->animating ? 0 : -1)) {
            ;
        }
        
        if (engine->destroyRequested) {
            LOGI("Engine thread destroy requested!");
            pthread_mutex_lock(&engine->mutex);
            engine_term_display(engine);
            if (engine->inputQueue != NULL) {
                AInputQueue_detachLooper(engine->inputQueue);
            }
            engine->destroyed = 1;
            pthread_cond_broadcast(&engine->cond);
            pthread_mutex_unlock(&engine->mutex);
            // Can't touch engine object after this.
            return NULL;   // EXIT THREAD
        }
        
        if (engine->animating) {
            // Done with events; draw next animation frame.
            engine->angle += .01f;
            if (engine->angle > 1) {
                engine->angle = 0;
            }
            engine_draw_frame(engine);
        }
    }
}

static struct engine* engine_create(ANativeActivity* activity) {
    struct engine* engine = (struct engine*)malloc(sizeof(struct engine));
    memset(engine, 0, sizeof(struct engine));
    engine->activity = activity;
    
    pthread_mutex_init(&engine->mutex, NULL);
    pthread_cond_init(&engine->cond, NULL);
    
    int msgpipe[2];
    if (pipe(msgpipe)) {
        LOGI("could not create pipe: %s", strerror(errno));
    }
    engine->msgread = msgpipe[0];
    engine->msgwrite = msgpipe[1];
    
    pthread_attr_t attr; 
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&engine->thread, &attr, engine_entry, engine);
    
    // Wait for thread to start.
    pthread_mutex_lock(&engine->mutex);
    while (!engine->running) {
        pthread_cond_wait(&engine->cond, &engine->mutex);
    }
    pthread_mutex_unlock(&engine->mutex);
    
    return engine;
}

static void engine_write_cmd(struct engine* engine, int8_t cmd) {
    if (write(engine->msgwrite, &cmd, sizeof(cmd)) != sizeof(cmd)) {
        LOGI("Failure writing engine cmd: %s\n", strerror(errno));
    }
}

static void engine_set_input(struct engine* engine, AInputQueue* inputQueue) {
    pthread_mutex_lock(&engine->mutex);
    engine->pendingInputQueue = inputQueue;
    engine_write_cmd(engine, ENGINE_CMD_INPUT_CHANGED);
    while (engine->inputQueue != engine->pendingInputQueue) {
        pthread_cond_wait(&engine->cond, &engine->mutex);
    }
    pthread_mutex_unlock(&engine->mutex);
}

static void engine_set_window(struct engine* engine, ANativeWindow* window) {
    pthread_mutex_lock(&engine->mutex);
    engine->pendingWindow = window;
    engine_write_cmd(engine, ENGINE_CMD_WINDOW_CHANGED);
    while (engine->window != engine->pendingWindow) {
        pthread_cond_wait(&engine->cond, &engine->mutex);
    }
    pthread_mutex_unlock(&engine->mutex);
}

static void engine_destroy(struct engine* engine) {
    pthread_mutex_lock(&engine->mutex);
    engine_write_cmd(engine, ENGINE_CMD_DESTROY);
    while (!engine->destroyed) {
        pthread_cond_wait(&engine->cond, &engine->mutex);
    }
    pthread_mutex_unlock(&engine->mutex);
    
    close(engine->msgread);
    close(engine->msgwrite);
    pthread_cond_destroy(&engine->cond);
    pthread_mutex_destroy(&engine->mutex);
    free(engine);
}

// --------------------------------------------------------------------
// Native activity interaction (called from main thread)
// --------------------------------------------------------------------

static void onDestroy(ANativeActivity* activity)
{
    LOGI("Destroy: %p\n", activity);
    engine_destroy((struct engine*)activity->instance);
}

static void onStart(ANativeActivity* activity)
{
    LOGI("Start: %p\n", activity);
}

static void onResume(ANativeActivity* activity)
{
    LOGI("Resume: %p\n", activity);
}

static void* onSaveInstanceState(ANativeActivity* activity, size_t* outLen)
{
    LOGI("SaveInstanceState: %p\n", activity);
    return NULL;
}

static void onPause(ANativeActivity* activity)
{
    LOGI("Pause: %p\n", activity);
}

static void onStop(ANativeActivity* activity)
{
    LOGI("Stop: %p\n", activity);
}

static void onLowMemory(ANativeActivity* activity)
{
    LOGI("LowMemory: %p\n", activity);
}

static void onWindowFocusChanged(ANativeActivity* activity, int focused)
{
    LOGI("WindowFocusChanged: %p -- %d\n", activity, focused);
    engine_write_cmd((struct engine*)activity->instance,
            focused ? ENGINE_CMD_GAINED_FOCUS : ENGINE_CMD_LOST_FOCUS);
}

static void onNativeWindowCreated(ANativeActivity* activity, ANativeWindow* window)
{
    LOGI("NativeWindowCreated: %p -- %p\n", activity, window);
    engine_set_window((struct engine*)activity->instance, window);
}

static void onNativeWindowDestroyed(ANativeActivity* activity, ANativeWindow* window)
{
    LOGI("NativeWindowDestroyed: %p -- %p\n", activity, window);
    engine_set_window((struct engine*)activity->instance, NULL);
}

static void onInputQueueCreated(ANativeActivity* activity, AInputQueue* queue)
{
    LOGI("InputQueueCreated: %p -- %p\n", activity, queue);
    engine_set_input((struct engine*)activity->instance, queue);
}

static void onInputQueueDestroyed(ANativeActivity* activity, AInputQueue* queue)
{
    LOGI("InputQueueDestroyed: %p -- %p\n", activity, queue);
    engine_set_input((struct engine*)activity->instance, NULL);
}

void ANativeActivity_onCreate(ANativeActivity* activity,
        void* savedState, size_t savedStateSize)
{
    LOGI("Creating: %p\n", activity);
    activity->callbacks->onDestroy = onDestroy;
    activity->callbacks->onStart = onStart;
    activity->callbacks->onResume = onResume;
    activity->callbacks->onSaveInstanceState = onSaveInstanceState;
    activity->callbacks->onPause = onPause;
    activity->callbacks->onStop = onStop;
    activity->callbacks->onLowMemory = onLowMemory;
    activity->callbacks->onWindowFocusChanged = onWindowFocusChanged;
    activity->callbacks->onNativeWindowCreated = onNativeWindowCreated;
    activity->callbacks->onNativeWindowDestroyed = onNativeWindowDestroyed;
    activity->callbacks->onInputQueueCreated = onInputQueueCreated;
    activity->callbacks->onInputQueueDestroyed = onInputQueueDestroyed;
    
    activity->instance = engine_create(activity);
}
