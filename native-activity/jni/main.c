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

#include <android/native_activity.h>
#include <android/log.h>

#include <jni.h>

#include <errno.h>
#include <poll.h>
#include <pthread.h>
#include <sched.h>
#include <string.h>
#include <sys/resource.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __VA_ARGS__))

struct engine {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    
    int msgread;
    int msgwrite;
    
    ANativeActivity* activity;
    pthread_t thread;
    
    int running;
    int destroyed;
    AInputQueue* inputQueue;
    ANativeWindow* window;
    AInputQueue* pendingInputQueue;
    ANativeWindow* pendingWindow;
};

enum {
    ENGINE_CMD_INPUT_CHANGED,
    ENGINE_CMD_WINDOW_CHANGED,
    ENGINE_CMD_DESTROY,
};

static void* engine_entry(void* param) {
    struct engine* engine = (struct engine*)param;
    struct pollfd pfd[2];
    int numfd;
    
    pthread_mutex_lock(&engine->mutex);
    engine->running = 1;
    pthread_cond_broadcast(&engine->cond);
    pthread_mutex_unlock(&engine->mutex);
    
    // loop waiting for stuff to do.  we wait for input events or
    // commands from the main thread.
    
    pfd[0].fd = engine->msgread;
    pfd[0].events = POLLIN;
    pfd[0].revents = 0;
    
    while (1) {
        if (engine->inputQueue != NULL) {
            numfd = 2;
            pfd[1].fd = AInputQueue_getFd(engine->inputQueue);
            pfd[1].events = POLLIN;
        } else {
            numfd = 1;
        }
        
        pfd[0].revents = 0;
        pfd[1].revents = 0;
        int nfd = poll(pfd, numfd, -1);
        if (nfd <= 0) {
            LOGI("Engine error in poll: %s\n", strerror(errno));
            // Should cleanly exit!
            continue;
        }
        
        if (pfd[0].revents == POLLIN) {
            int8_t cmd;
            if (read(engine->msgread, &cmd, sizeof(cmd)) == sizeof(cmd)) {
                switch (cmd) {
                    case ENGINE_CMD_INPUT_CHANGED:
                        LOGI("Engine: ENGINE_CMD_INPUT_CHANGED\n");
                        pthread_mutex_lock(&engine->mutex);
                        engine->inputQueue = engine->pendingInputQueue;
                        pthread_cond_broadcast(&engine->cond);
                        pthread_mutex_unlock(&engine->mutex);
                        break;
                    case ENGINE_CMD_WINDOW_CHANGED:
                        LOGI("Engine: ENGINE_CMD_WINDOW_CHANGED\n");
                        pthread_mutex_lock(&engine->mutex);
                        engine->window = engine->pendingWindow;
                        pthread_cond_broadcast(&engine->cond);
                        pthread_mutex_unlock(&engine->mutex);
                        break;
                    case ENGINE_CMD_DESTROY:
                        LOGI("Engine: ENGINE_CMD_DESTROY\n");
                        pthread_mutex_lock(&engine->mutex);
                        engine->destroyed = 1;
                        pthread_cond_broadcast(&engine->cond);
                        pthread_mutex_unlock(&engine->mutex);
                        // Can't touch engine object after this.
                        return NULL;   // EXIT THREAD
                }
            } else {
                LOGI("Failure reading engine cmd: %s\n", strerror(errno));
            }
            
        } else if (engine->inputQueue != NULL && pfd[1].revents == POLLIN) {
            AInputEvent* event = NULL;
            if (AInputQueue_getEvent(engine->inputQueue, &event) >= 0) {
                LOGI("New input event: type=%d\n", AInputEvent_getType(event));
                AInputQueue_finishEvent(engine->inputQueue, event, 0);
            } else {
                LOGI("Failure reading next input event: %s\n", strerror(errno));
            }
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
}

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
}

static void onNativeWindowCreated(ANativeActivity* activity, ANativeWindow* window)
{
    LOGI("NativeWindowCreated: %p -- %p\n", activity, window);
    engine_set_window((struct engine*)activity->instance, window);
}

static void onNativeWindowChanged(ANativeActivity* activity, ANativeWindow* window)
{
    LOGI("NativeWindowChanged: %p -- %p\n", activity, window);
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
    activity->callbacks->onNativeWindowChanged = onNativeWindowChanged;
    activity->callbacks->onNativeWindowDestroyed = onNativeWindowDestroyed;
    activity->callbacks->onInputQueueCreated = onInputQueueCreated;
    activity->callbacks->onInputQueueDestroyed = onInputQueueDestroyed;
    
    activity->instance = engine_create(activity);
}
