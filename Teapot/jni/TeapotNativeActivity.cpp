/*
 * Copyright 2013 The Android Open Source Project
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

//--------------------------------------------------------------------------------
// Include files
//--------------------------------------------------------------------------------
#include <jni.h>
#include <errno.h>

#include <vector>
#include <EGL/egl.h>
#include <GLES/gl.h>

#include <android/sensor.h>
#include <android/log.h>
#include <android_native_app_glue.h>
#include <android/native_window_jni.h>
#include <cpu-features.h>

#include "NDKSupport/NDKSupport.h"
#include "TeapotRenderer.h"

/**
 * Our saved state data.
 */
struct saved_state {
    float angle;
    int32_t x;
    int32_t y;
};

/**
 * Shared state for our app.
 */
class engine {
public:
    struct android_app* app;

    ASensorManager* sensorManager;
    const ASensor* accelerometerSensor;
    ASensorEventQueue* sensorEventQueue;

    int animating;
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    int32_t width;
    int32_t height;
    struct saved_state state;

    std::vector<int32_t> _vecPointers;
    tapCamera _tapCamera;
    TeapotRenderer _renderer;

    DoubletapDetector _doubletapDetector;
    perfMonitor _monitor;
};


void showUI(android_app* app)
{
    JNIEnv *jni;
    app->activity->vm->AttachCurrentThread(&jni, NULL);

    //Default class retrieval
    jclass clazz = jni->GetObjectClass(app->activity->clazz);
    jmethodID methodID = jni->GetMethodID(clazz, "showUI", "()V");
    jni->CallVoidMethod(app->activity->clazz, methodID);

    app->activity->vm->DetachCurrentThread();

    return;
}


void updateFPS(android_app* app, float fFPS)
{
    JNIEnv *jni;
    app->activity->vm->AttachCurrentThread(&jni, NULL);

    //Default class retrieval
    jclass clazz = jni->GetObjectClass(app->activity->clazz);
    jmethodID methodID = jni->GetMethodID(clazz, "updateFPS", "(F)V");
    jni->CallVoidMethod(app->activity->clazz, methodID, fFPS);

    app->activity->vm->DetachCurrentThread();

    return;
}

/**
 * Initialize an EGL context for the current display.
 */
static int engine_init_display(struct engine* engine) {
    // initialize OpenGL ES and EGL

    showUI(engine->app);

    const EGLint contextAttribs[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2,  //Request opengl ES2.0
            EGL_NONE
    };

    EGLint w, h, dummy, format;
    EGLint numConfigs;
    EGLConfig config;
    EGLSurface surface;
    EGLContext context;

    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    eglInitialize(display, 0, 0);

    /*
     * Here specify the attributes of the desired configuration.
     * Below, we select an EGLConfig with at least 8 bits per color
     * component compatible with on-screen windows
     */
    const EGLint attribs[] = {
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,    //Request opengl ES2.0
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_DEPTH_SIZE, 24,
            EGL_NONE
    };
    eglChooseConfig(display, attribs, &config, 1, &numConfigs);
    if( !numConfigs )
    {
        //Fall back to 16bit depth buffer
        const EGLint attribs[] = {
                EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,    //Request opengl ES2.0
                EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                EGL_BLUE_SIZE, 8,
                EGL_GREEN_SIZE, 8,
                EGL_RED_SIZE, 8,
                EGL_DEPTH_SIZE, 16,
                EGL_NONE
        };
        eglChooseConfig(display, attribs, &config, 1, &numConfigs);
    }
    if ( !numConfigs )
    {
        LOGW("Unable to retrieve EGL config");
        return -1;
    }

    /* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
     * guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
     * As soon as we picked a EGLConfig, we can safely reconfigure the
     * ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

    ANativeWindow_setBuffersGeometry(engine->app->window, 0, 0, format);

    surface = eglCreateWindowSurface(display, config, engine->app->window, NULL);
    context = eglCreateContext(display, config, NULL, contextAttribs);

    if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
        LOGW("Unable to eglMakeCurrent");
        return -1;
    }

    eglQuerySurface(display, surface, EGL_WIDTH, &w);
    eglQuerySurface(display, surface, EGL_HEIGHT, &h);

    engine->display = display;
    engine->context = context;
    engine->surface = surface;
    engine->width = w;
    engine->height = h;
    engine->state.angle = 0;

    // Initialize GL state.
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glFrontFace(GL_CW);
    glViewport(0, 0, engine->width, engine->height);

    engine->_tapCamera.setFlip(1.f, -1.f, -8.f);

    engine->_renderer.init();
    engine->_renderer.bind(&engine->_tapCamera);

    return 0;
}

/**
 * Just the current frame in the display.
 */
static void engine_draw_frame(struct engine* engine) {
    if (engine->display == NULL) {
        // No display.
        return;
    }

    float fFPS;
    bool b =  engine->_monitor.update(fFPS);
    if( b )
    {
        updateFPS( engine->app, fFPS );
    }

    struct timeval Time;
    gettimeofday( &Time, NULL );
    double dTime = Time.tv_sec + Time.tv_usec * 1.0/1000000.0 ;

    engine->_renderer.update(dTime);

    // Just fill the screen with a color.
    glViewport(0, 0, engine->width, engine->height);
    glClearColor(0.5f, 0.5f, 0.5f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    engine->_renderer.render();

    eglSwapBuffers(engine->display, engine->surface);
}

/**
 * Tear down the EGL context currently associated with the display.
 */
static void engine_term_display(struct engine* engine) {

    engine->_renderer.unload();

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

int32_t findIndex( AInputEvent* event, int32_t iID )
{
    int32_t iCount = AMotionEvent_getPointerCount(event);
    for( int32_t i = 0; i < iCount; ++i )
    {
        if( iID == AMotionEvent_getPointerId(event, i) )
            return i;
    }
    return -1;
}

void updatePosition( engine* engine, AInputEvent* event, int32_t iIndex, float& fX, float& fY)
{
    engine->state.x = AMotionEvent_getX(event, iIndex);
    engine->state.y = AMotionEvent_getY(event, iIndex);

    fX = 2.0f * engine->state.x / engine->width -1.f;
    fY = 2.0f * engine->state.y / engine->height -1.f;
}

/**
 * Process the next input event.
 */
static int32_t engine_handle_input(struct android_app* app, AInputEvent* event) {
    struct engine* engine = (struct engine*)app->userData;
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        engine->animating = 1;

        int32_t iCount = AMotionEvent_getPointerCount(event);
        int32_t iAction = AMotionEvent_getAction(event);
        unsigned int flags = iAction & AMOTION_EVENT_ACTION_MASK;
        float fX;
        float fY;
        float fX2;
        float fY2;
        switch( flags )
        {
        case AMOTION_EVENT_ACTION_DOWN:
            engine->_vecPointers.push_back(AMotionEvent_getPointerId(event, 0));

            //Single touch
            if( engine->_doubletapDetector.detect(event) )
            {
                //Detect double tap
                engine->_tapCamera.reset(true);
            }
            else
            {
                //Otherwise, start dragging
                updatePosition(engine, event, 0, fX, fY);
                engine->_tapCamera.beginDrag( vec2( fX, fY ) );
            }
            break;
        case AMOTION_EVENT_ACTION_POINTER_DOWN:
        {
            int32_t iIndex = (iAction & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
            engine->_vecPointers.push_back(AMotionEvent_getPointerId(event, iIndex));
            if( iCount == 2 )
            {
                //Start pinch
                //Start new pinch
                int32_t iIndex = findIndex( event, engine->_vecPointers[0] );
                updatePosition(engine, event, iIndex, fX, fY);
                iIndex = findIndex( event, engine->_vecPointers[1] );
                updatePosition(engine, event, iIndex, fX2, fY2);
                engine->_tapCamera.beginPinch( vec2( fX, fY ), vec2( fX2, fY2 ) );
            }
        }
            break;
        case AMOTION_EVENT_ACTION_UP:
            //Update doubletap detector
            engine->_doubletapDetector.detect(event);

            engine->_vecPointers.pop_back();
            engine->_tapCamera.endDrag();
            break;
        case AMOTION_EVENT_ACTION_POINTER_UP:
        {
            int32_t iIndex = (iAction & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
            int32_t iReleasedPointerID = AMotionEvent_getPointerId(event, iIndex);

            std::vector<int32_t>::iterator it = engine->_vecPointers.begin();
            std::vector<int32_t>::iterator itEnd = engine->_vecPointers.end();
            int32_t i = 0;
            for(;it!=itEnd;++it, ++i)
            {
                if( *it == iReleasedPointerID )
                {
                    engine->_vecPointers.erase(it);
                    break;
                }
            }

            if( i <= 1 )
            {
                //Reset pinch or drag
                if( iCount == 2 )
                {
                    //Start new drag
                    int32_t iIndex = findIndex( event, engine->_vecPointers.front() );
                    updatePosition(engine, event, iIndex, fX, fY);
                    engine->_tapCamera.beginDrag( vec2( fX, fY ) );
                }
                else
                {
                    //Start new pinch
                    int32_t iIndex = findIndex( event, engine->_vecPointers[0] );
                    updatePosition(engine, event, iIndex, fX, fY);
                    iIndex = findIndex( event, engine->_vecPointers[1] );
                    updatePosition(engine, event, iIndex, fX2, fY2);
                    engine->_tapCamera.beginPinch( vec2( fX, fY ), vec2( fX2, fY2 ) );
                }
            }
        }
            break;
        case AMOTION_EVENT_ACTION_MOVE:
        {
            switch(iCount)
            {
            case 1:
            {
                //Single touch
                int32_t iIndex = findIndex( event, engine->_vecPointers.front() );
                updatePosition(engine, event, iIndex, fX, fY);
                engine->_tapCamera.drag( vec2( fX, fY ) );
            }
                break;
            default:
            {
                //Multi touch
                int32_t iIndex = findIndex( event, engine->_vecPointers[0] );
                updatePosition(engine, event, iIndex, fX, fY);
                iIndex = findIndex( event, engine->_vecPointers[1] );
                updatePosition(engine, event, iIndex, fX2, fY2);
                engine->_tapCamera.pinch( vec2( fX, fY ), vec2( fX2, fY2 ) );
            }
                break;
            }
            break;
        }
        case AMOTION_EVENT_ACTION_CANCEL:
            break;
        }
        return 1;
    }
    return 0;
}

/**
 * Process the next main command.
 */
static void engine_handle_cmd(struct android_app* app, int32_t cmd) {
    struct engine* engine = (struct engine*)app->userData;
    switch (cmd) {
        case APP_CMD_SAVE_STATE:
            // The system has asked us to save our current state.  Do so.
            engine->app->savedState = malloc(sizeof(struct saved_state));
            *((struct saved_state*)engine->app->savedState) = engine->state;
            engine->app->savedStateSize = sizeof(struct saved_state);
            break;
        case APP_CMD_INIT_WINDOW:
            // The window is being shown, get it ready.
            if (engine->app->window != NULL) {
                engine_init_display(engine);
                engine_draw_frame(engine);
                engine->animating = 1;
            }
            break;
        case APP_CMD_TERM_WINDOW:
            // The window is being hidden or closed, clean it up.
            engine_term_display(engine);
            break;
        case APP_CMD_GAINED_FOCUS:
            // When our app gains focus, we start monitoring the accelerometer.
            if (engine->accelerometerSensor != NULL) {
                ASensorEventQueue_enableSensor(engine->sensorEventQueue,
                        engine->accelerometerSensor);
                // We'd like to get 60 events per second (in us).
                ASensorEventQueue_setEventRate(engine->sensorEventQueue,
                        engine->accelerometerSensor, (1000L/60)*1000);
            }
            break;
        case APP_CMD_LOST_FOCUS:
            // When our app loses focus, we stop monitoring the accelerometer.
            // This is to avoid consuming battery while not being used.
            if (engine->accelerometerSensor != NULL) {
                ASensorEventQueue_disableSensor(engine->sensorEventQueue,
                        engine->accelerometerSensor);
            }
            // Also stop animating.
            engine->animating = 0;
            engine_draw_frame(engine);
            break;
    }
}

/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 */
void android_main(struct android_app* state) {
    engine engine;

    // Make sure glue isn't stripped.
    app_dummy();

    state->userData = &engine;
    state->onAppCmd = engine_handle_cmd;
    state->onInputEvent = engine_handle_input;
    engine.app = state;
    engine._doubletapDetector.setConfiguration(state->config);

    //Init helper functions
    JNIHelper::init( state->activity );


    // Prepare to monitor accelerometer
    engine.sensorManager = ASensorManager_getInstance();
    engine.accelerometerSensor = ASensorManager_getDefaultSensor(engine.sensorManager,
            ASENSOR_TYPE_ACCELEROMETER);
    engine.sensorEventQueue = ASensorManager_createEventQueue(engine.sensorManager,
            state->looper, LOOPER_ID_USER, NULL, NULL);

    if (state->savedState != NULL) {
        // We are starting with a previous saved state; restore from it.
        engine.state = *(struct saved_state*)state->savedState;
    }

    // loop waiting for stuff to do.

    while (1) {
        // Read all pending events.
        int ident;
        int events;
        struct android_poll_source* source;

        // If not animating, we will block forever waiting for events.
        // If animating, we loop until all events are read, then continue
        // to draw the next frame of animation.
        while ((ident=ALooper_pollAll(engine.animating ? 0 : -1, NULL, &events,
                (void**)&source)) >= 0) {

            // Process this event.
            if (source != NULL) {
                source->process(state, source);
            }

            // If a sensor has data, process it now.
            if (ident == LOOPER_ID_USER) {
                if (engine.accelerometerSensor != NULL) {
                    ASensorEvent event;
                    while (ASensorEventQueue_getEvents(engine.sensorEventQueue,
                            &event, 1) > 0) {
                    }
                }
            }

            // Check if we are exiting.
            if (state->destroyRequested != 0) {
                engine_term_display(&engine);
                return;
            }
        }

        if (engine.animating) {
            // Done with events; draw next animation frame.
            engine.state.angle += .01f;
            if (engine.state.angle > 1) {
                engine.state.angle = 0;
            }

            // Drawing is throttled to the screen update rate, so there
            // is no need to do timing here.
            engine_draw_frame(&engine);
        }
    }
}
