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

#include "MoreTeapotsRenderer.h"

const int32_t NUM_TEAPOTS_X = 8;
const int32_t NUM_TEAPOTS_Y = 8;
const int32_t NUM_TEAPOTS_Z = 6;

//-------------------------------------------------------------------------
//Shared state for our app.
//-------------------------------------------------------------------------
struct android_app;
class engine {
    MoreTeapotsRenderer _renderer;

    GLContext* _glContext;

    bool _bInitializedResources;
    bool _bHasFocus;

    DoubletapDetector _doubletapDetector;
    PinchDetector _pinchDetector;
    DragDetector _dragDetector;
    perfMonitor _monitor;

    tapCamera _tapCamera;

    android_app* _app;

    ASensorManager* _sensorManager;
    const ASensor* _accelerometerSensor;
    ASensorEventQueue* _sensorEventQueue;

    void updateFPS(float fFPS);
    void showUI();
    void transformPosition( vec2& vec );

public:
    static void handleCmd(struct android_app* app, int32_t cmd);
    static int32_t handleInput( android_app* app, AInputEvent* event );

    engine();
    ~engine();
    void setState(android_app* state);
    int initDisplay();
    void loadResources();
    void unloadResources();
    void drawFrame();
    void termDisplay();
    void trimMemory();
    bool isReady();

    void updatePosition( AInputEvent* event, int32_t iIndex, float& fX, float& fY);

    void initSensors();
    void processSensors( int32_t id );
    void suspendSensors();
    void resumeSensors();
};

//-------------------------------------------------------------------------
//Ctor
//-------------------------------------------------------------------------
engine::engine() :
        _bInitializedResources( false ),
        _bHasFocus( false ),
        _app( NULL ),
        _sensorManager( NULL ),
        _accelerometerSensor( NULL ),
        _sensorEventQueue( NULL )
{
    _glContext = GLContext::getInstance();
}

//-------------------------------------------------------------------------
//Dtor
//-------------------------------------------------------------------------
engine::~engine()
{
}

/**
 * Load resources
 */
void engine::loadResources()
{
    _renderer.init(NUM_TEAPOTS_X, NUM_TEAPOTS_Y, NUM_TEAPOTS_Z);
    _renderer.bind(&_tapCamera);
}

/**
 * Unload resources
 */
void engine::unloadResources()
{
    _renderer.unload();
}

/**
 * Initialize an EGL context for the current display.
 */
int engine::initDisplay()
{
    if( !_bInitializedResources )
    {
        _glContext->init( _app->window );
        loadResources();
        _bInitializedResources = true;
    }
    else
    {
        // initialize OpenGL ES and EGL
        if( EGL_SUCCESS != _glContext->resume( _app->window ) )
        {
            unloadResources();
            loadResources();
        }
    }

    showUI();

    // Initialize GL state.
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    //Note that screen size might have been changed
    glViewport(0, 0,
            _glContext->getScreenWidth(),
            _glContext->getScreenHeight() );
    _renderer.updateViewport();

    _tapCamera.setFlip(1.f, -1.f, -1.f);
    _tapCamera.setPinchTransformFactor(10.f, 10.f, 8.f);

    return 0;
}

/**
 * Just the current frame in the display.
 */
void engine::drawFrame()
{
    float fFPS;
    if( _monitor.update(fFPS) )
    {
        updateFPS( fFPS );
    }
    double dTime = _monitor.getCurrentTime();
    _renderer.update(dTime);

    // Just fill the screen with a color.
    glClearColor(0.5f, 0.5f, 0.5f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    _renderer.render();

    // Swap
    if( EGL_SUCCESS != _glContext->swap() )
    {
        unloadResources();
        loadResources();
    }
}

/**
 * Tear down the EGL context currently associated with the display.
 */
void engine::termDisplay()
{
    _glContext->suspend();

}

void engine::trimMemory()
{
    LOGI( "Trimming memory" );
    _glContext->invalidate();
}
/**
 * Process the next input event.
 */
int32_t engine::handleInput( android_app* app, AInputEvent* event )
{
    engine* eng = (engine*)app->userData;
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION)
    {
        GESTURE_STATE doubleTapState = eng->_doubletapDetector.detect(event);
        GESTURE_STATE dragState = eng->_dragDetector.detect(event);
        GESTURE_STATE pinchState = eng->_pinchDetector.detect(event);

        //Double tap detector has a priority over other detectors
         if( doubleTapState == GESTURE_STATE_ACTION )
         {
             //Detect double tap
             eng->_tapCamera.reset(true);
         }
         else
         {
             //Handle drag state
             if( dragState & GESTURE_STATE_START )
             {
                 //Otherwise, start dragging
                 vec2 v;
                 eng->_dragDetector.getPointer( v );
                 eng->transformPosition( v );
                 eng->_tapCamera.beginDrag( v );
             }
             else if( dragState & GESTURE_STATE_MOVE )
             {
                 vec2 v;
                 eng->_dragDetector.getPointer( v );
                 eng->transformPosition( v );
                 eng->_tapCamera.drag( v );
             }
             else if( dragState & GESTURE_STATE_END )
             {
                 eng->_tapCamera.endDrag();
             }

             //Handle pinch state
             if( pinchState & GESTURE_STATE_START )
             {
                 //Start new pinch
                 vec2 v1;
                 vec2 v2;
                 eng->_pinchDetector.getPointers( v1, v2 );
                 eng->transformPosition( v1 );
                 eng->transformPosition( v2 );
                 eng->_tapCamera.beginPinch( v1, v2 );
             }
             else if( pinchState & GESTURE_STATE_MOVE )
             {
                 //Multi touch
                 //Start new pinch
                 vec2 v1;
                 vec2 v2;
                 eng->_pinchDetector.getPointers( v1, v2 );
                 eng->transformPosition( v1 );
                 eng->transformPosition( v2 );
                 eng->_tapCamera.pinch( v1, v2 );
             }
         }
         return 1;
    }
    return 0;
}

/**
 * Process the next main command.
 */
void engine::handleCmd(struct android_app* app, int32_t cmd)
{
    engine* eng = (engine*)app->userData;
    switch (cmd) {
        case APP_CMD_SAVE_STATE:
            break;
        case APP_CMD_INIT_WINDOW:
            // The window is being shown, get it ready.
            if (app->window != NULL) {
                eng->initDisplay();
                eng->drawFrame();
            }
            break;
        case APP_CMD_TERM_WINDOW:
            // The window is being hidden or closed, clean it up.
            eng->termDisplay();
            eng->_bHasFocus = false;
            break;
        case APP_CMD_STOP:
            break;
        case APP_CMD_GAINED_FOCUS:
            eng->resumeSensors();
            //Start animation
            eng->_bHasFocus = true;
            break;
        case APP_CMD_LOST_FOCUS:
            eng->suspendSensors();
            // Also stop animating.
            eng->_bHasFocus = false;
            eng->drawFrame();
            break;
        case APP_CMD_LOW_MEMORY:
            //Free up GL resources
            eng->trimMemory();
            break;
    }
}

//-------------------------------------------------------------------------
//Sensor handlers
//-------------------------------------------------------------------------
void engine::initSensors()
{
    _sensorManager = ASensorManager_getInstance();
    _accelerometerSensor = ASensorManager_getDefaultSensor( _sensorManager,
            ASENSOR_TYPE_ACCELEROMETER);
    _sensorEventQueue = ASensorManager_createEventQueue( _sensorManager,
            _app->looper, LOOPER_ID_USER, NULL, NULL);
}

void engine::processSensors( int32_t id )
{
    // If a sensor has data, process it now.
    if( id == LOOPER_ID_USER )
    {
        if (_accelerometerSensor != NULL)
        {
            ASensorEvent event;
            while (ASensorEventQueue_getEvents(_sensorEventQueue,
                    &event, 1) > 0)
            {
            }
        }
    }
}

void engine::resumeSensors()
{
    // When our app gains focus, we start monitoring the accelerometer.
    if (_accelerometerSensor != NULL) {
        ASensorEventQueue_enableSensor(_sensorEventQueue,
                _accelerometerSensor);
        // We'd like to get 60 events per second (in us).
        ASensorEventQueue_setEventRate(_sensorEventQueue,
                _accelerometerSensor, (1000L/60)*1000);
    }
}

void engine::suspendSensors()
{
    // When our app loses focus, we stop monitoring the accelerometer.
    // This is to avoid consuming battery while not being used.
    if (_accelerometerSensor != NULL) {
        ASensorEventQueue_disableSensor(_sensorEventQueue,
                _accelerometerSensor);
    }
}


//-------------------------------------------------------------------------
//Misc
//-------------------------------------------------------------------------
void engine::setState(android_app* state)
{
    _app = state;
    _doubletapDetector.setConfiguration( _app->config );
    _dragDetector.setConfiguration( _app->config );
    _pinchDetector.setConfiguration( _app->config );
}

bool engine::isReady()
{
    if( _bHasFocus )
        return true;

    return false;
}

void engine::transformPosition( vec2& vec )
{
    vec = vec2( 2.0f, 2.0f ) * vec / vec2( _glContext->getScreenWidth(), _glContext->getScreenHeight() ) - vec2( 1.f, 1.f );
}

void engine::showUI()
{
    JNIEnv *jni;
    _app->activity->vm->AttachCurrentThread(&jni, NULL);

    //Default class retrieval
    jclass clazz = jni->GetObjectClass( _app->activity->clazz );
    jmethodID methodID = jni->GetMethodID(clazz, "showUI", "()V" );
    jni->CallVoidMethod( _app->activity->clazz, methodID );

    _app->activity->vm->DetachCurrentThread();
    return;
}

void engine::updateFPS(float fFPS)
{
    JNIEnv *jni;
    _app->activity->vm->AttachCurrentThread(&jni, NULL);

    //Default class retrieval
    jclass clazz = jni->GetObjectClass(_app->activity->clazz);
    jmethodID methodID = jni->GetMethodID(clazz, "updateFPS", "(F)V");
    jni->CallVoidMethod(_app->activity->clazz, methodID, fFPS);

    _app->activity->vm->DetachCurrentThread();
    return;
}

engine g_engine;

/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 */
void android_main(android_app* state)
{
    app_dummy();

    g_engine.setState( state );

    //Init helper functions
    JNIHelper::init( state->activity );

    state->userData = &g_engine;
    state->onAppCmd = engine::handleCmd;
    state->onInputEvent = engine::handleInput;

#ifdef USE_NDK_PROFILER
    monstartup("libNativeActivity.so");
#endif

    // Prepare to monitor accelerometer
    g_engine.initSensors();

    // loop waiting for stuff to do.
    while (1) {
        // Read all pending events.
        int id;
        int events;
        android_poll_source* source;

        // If not animating, we will block forever waiting for events.
        // If animating, we loop until all events are read, then continue
        // to draw the next frame of animation.
        while ((id = ALooper_pollAll( g_engine.isReady() ? 0 : -1, NULL, &events,
                (void**)&source) ) >= 0)
        {
            // Process this event.
            if (source != NULL)
                source->process(state, source);

            g_engine.processSensors( id );

            // Check if we are exiting.
            if (state->destroyRequested != 0)
            {
                g_engine.termDisplay();
                return;
            }
        }

        if( g_engine.isReady() )
        {
            // Drawing is throttled to the screen update rate, so there
            // is no need to do timing here.
            g_engine.drawFrame();
        }
    }
}

