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

//-------------------------------------------------------------------------
//Preprocessor
//-------------------------------------------------------------------------
#define HELPER_CLASS_NAME "com/sample/helper/NDKHelper" //Class name of helper function
//-------------------------------------------------------------------------
//Constants
//-------------------------------------------------------------------------
const int32_t NUM_TEAPOTS_X = 8;
const int32_t NUM_TEAPOTS_Y = 8;
const int32_t NUM_TEAPOTS_Z = 8;

//-------------------------------------------------------------------------
//Shared state for our app.
//-------------------------------------------------------------------------
struct android_app;
class Engine
{
    MoreTeapotsRenderer renderer_;

    ndk_helper::GLContext* gl_context_;

    bool initialized_resources_;
    bool has_focus_;

    ndk_helper::DoubletapDetector doubletap_detector_;
    ndk_helper::PinchDetector pinch_detector_;
    ndk_helper::DragDetector drag_detector_;
    ndk_helper::PerfMonitor monitor_;

    ndk_helper::TapCamera tap_camera_;

    android_app* app_;

    ASensorManager* sensor_manager_;
    const ASensor* accelerometer_sensor_;
    ASensorEventQueue* sensor_event_queue_;

    void UpdateFPS( float fps );
    void ShowUI();
    void TransformPosition( ndk_helper::Vec2& vec );

public:
    static void HandleCmd( struct android_app* app,
            int32_t cmd );
    static int32_t HandleInput( android_app* app,
            AInputEvent* event );

    Engine();
    ~Engine();
    void SetState( android_app* state );
    int InitDisplay();
    void LoadResources();
    void UnloadResources();
    void DrawFrame();
    void TermDisplay();
    void TrimMemory();
    bool IsReady();

    void UpdatePosition( AInputEvent* event,
            int32_t index,
            float& x,
            float& y );

    void InitSensors();
    void ProcessSensors( int32_t id );
    void SuspendSensors();
    void ResumeSensors();
};

//-------------------------------------------------------------------------
//Ctor
//-------------------------------------------------------------------------
Engine::Engine() :
                initialized_resources_( false ),
                has_focus_( false ),
                app_( NULL ),
                sensor_manager_( NULL ),
                accelerometer_sensor_( NULL ),
                sensor_event_queue_( NULL )
{
    gl_context_ = ndk_helper::GLContext::GetInstance();
}

//-------------------------------------------------------------------------
//Dtor
//-------------------------------------------------------------------------
Engine::~Engine()
{
}

/**
 * Load resources
 */
void Engine::LoadResources()
{
    renderer_.Init( NUM_TEAPOTS_X, NUM_TEAPOTS_Y, NUM_TEAPOTS_Z );
    renderer_.Bind( &tap_camera_ );
}

/**
 * Unload resources
 */
void Engine::UnloadResources()
{
    renderer_.Unload();
}

/**
 * Initialize an EGL context for the current display.
 */
int Engine::InitDisplay()
{
    if( !initialized_resources_ )
    {
        gl_context_->Init( app_->window );
        LoadResources();
        initialized_resources_ = true;
    }
    else
    {
        // initialize OpenGL ES and EGL
        if( EGL_SUCCESS != gl_context_->Resume( app_->window ) )
        {
            UnloadResources();
            LoadResources();
        }
    }

    ShowUI();

    // Initialize GL state.
    glEnable( GL_CULL_FACE );
    glEnable( GL_DEPTH_TEST );
    glDepthFunc( GL_LEQUAL );

    //Note that screen size might have been changed
    glViewport( 0, 0, gl_context_->GetScreenWidth(), gl_context_->GetScreenHeight() );
    renderer_.UpdateViewport();

    tap_camera_.SetFlip( 1.f, -1.f, -1.f );
    tap_camera_.SetPinchTransformFactor( 10.f, 10.f, 8.f );

    return 0;
}

/**
 * Just the current frame in the display.
 */
void Engine::DrawFrame()
{
    float fps;
    if( monitor_.Update( fps ) )
    {
        UpdateFPS( fps );
    }
    double dTime = monitor_.GetCurrentTime();
    renderer_.Update( dTime );

    // Just fill the screen with a color.
    glClearColor( 0.5f, 0.5f, 0.5f, 1.f );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    renderer_.Render();

    // Swap
    if( EGL_SUCCESS != gl_context_->Swap() )
    {
        UnloadResources();
        LoadResources();
    }
}

/**
 * Tear down the EGL context currently associated with the display.
 */
void Engine::TermDisplay()
{
    gl_context_->Suspend();

}

void Engine::TrimMemory()
{
    LOGI( "Trimming memory" );
    gl_context_->Invalidate();
}
/**
 * Process the next input event.
 */
int32_t Engine::HandleInput( android_app* app,
        AInputEvent* event )
{
    Engine* eng = (Engine*) app->userData;
    if( AInputEvent_getType( event ) == AINPUT_EVENT_TYPE_MOTION )
    {
        ndk_helper::GESTURE_STATE doubleTapState = eng->doubletap_detector_.Detect( event );
        ndk_helper::GESTURE_STATE dragState = eng->drag_detector_.Detect( event );
        ndk_helper::GESTURE_STATE pinchState = eng->pinch_detector_.Detect( event );

        //Double tap detector has a priority over other detectors
        if( doubleTapState == ndk_helper::GESTURE_STATE_ACTION )
        {
            //Detect double tap
            eng->tap_camera_.Reset( true );
        }
        else
        {
            //Handle drag state
            if( dragState & ndk_helper::GESTURE_STATE_START )
            {
                //Otherwise, start dragging
                ndk_helper::Vec2 v;
                eng->drag_detector_.GetPointer( v );
                eng->TransformPosition( v );
                eng->tap_camera_.BeginDrag( v );
            }
            else if( dragState & ndk_helper::GESTURE_STATE_MOVE )
            {
                ndk_helper::Vec2 v;
                eng->drag_detector_.GetPointer( v );
                eng->TransformPosition( v );
                eng->tap_camera_.Drag( v );
            }
            else if( dragState & ndk_helper::GESTURE_STATE_END )
            {
                eng->tap_camera_.EndDrag();
            }

            //Handle pinch state
            if( pinchState & ndk_helper::GESTURE_STATE_START )
            {
                //Start new pinch
                ndk_helper::Vec2 v1;
                ndk_helper::Vec2 v2;
                eng->pinch_detector_.GetPointers( v1, v2 );
                eng->TransformPosition( v1 );
                eng->TransformPosition( v2 );
                eng->tap_camera_.BeginPinch( v1, v2 );
            }
            else if( pinchState & ndk_helper::GESTURE_STATE_MOVE )
            {
                //Multi touch
                //Start new pinch
                ndk_helper::Vec2 v1;
                ndk_helper::Vec2 v2;
                eng->pinch_detector_.GetPointers( v1, v2 );
                eng->TransformPosition( v1 );
                eng->TransformPosition( v2 );
                eng->tap_camera_.Pinch( v1, v2 );
            }
        }
        return 1;
    }
    return 0;
}

/**
 * Process the next main command.
 */
void Engine::HandleCmd( struct android_app* app,
        int32_t cmd )
{
    Engine* eng = (Engine*) app->userData;
    switch( cmd )
    {
    case APP_CMD_SAVE_STATE:
        break;
    case APP_CMD_INIT_WINDOW:
        // The window is being shown, get it ready.
        if( app->window != NULL )
        {
            eng->InitDisplay();
            eng->DrawFrame();
        }
        break;
    case APP_CMD_TERM_WINDOW:
        // The window is being hidden or closed, clean it up.
        eng->TermDisplay();
        eng->has_focus_ = false;
        break;
    case APP_CMD_STOP:
        break;
    case APP_CMD_GAINED_FOCUS:
        eng->ResumeSensors();
        //Start animation
        eng->has_focus_ = true;
        break;
    case APP_CMD_LOST_FOCUS:
        eng->SuspendSensors();
        // Also stop animating.
        eng->has_focus_ = false;
        eng->DrawFrame();
        break;
    case APP_CMD_LOW_MEMORY:
        //Free up GL resources
        eng->TrimMemory();
        break;
    }
}

//-------------------------------------------------------------------------
//Sensor handlers
//-------------------------------------------------------------------------
void Engine::InitSensors()
{
    sensor_manager_ = ASensorManager_getInstance();
    accelerometer_sensor_ = ASensorManager_getDefaultSensor( sensor_manager_,
            ASENSOR_TYPE_ACCELEROMETER );
    sensor_event_queue_ = ASensorManager_createEventQueue( sensor_manager_, app_->looper,
            LOOPER_ID_USER, NULL, NULL );
}

void Engine::ProcessSensors( int32_t id )
{
    // If a sensor has data, process it now.
    if( id == LOOPER_ID_USER )
    {
        if( accelerometer_sensor_ != NULL )
        {
            ASensorEvent event;
            while( ASensorEventQueue_getEvents( sensor_event_queue_, &event, 1 ) > 0 )
            {
            }
        }
    }
}

void Engine::ResumeSensors()
{
    // When our app gains focus, we start monitoring the accelerometer.
    if( accelerometer_sensor_ != NULL )
    {
        ASensorEventQueue_enableSensor( sensor_event_queue_, accelerometer_sensor_ );
        // We'd like to get 60 events per second (in us).
        ASensorEventQueue_setEventRate( sensor_event_queue_, accelerometer_sensor_,
                (1000L / 60) * 1000 );
    }
}

void Engine::SuspendSensors()
{
    // When our app loses focus, we stop monitoring the accelerometer.
    // This is to avoid consuming battery while not being used.
    if( accelerometer_sensor_ != NULL )
    {
        ASensorEventQueue_disableSensor( sensor_event_queue_, accelerometer_sensor_ );
    }
}

//-------------------------------------------------------------------------
//Misc
//-------------------------------------------------------------------------
void Engine::SetState( android_app* state )
{
    app_ = state;
    doubletap_detector_.SetConfiguration( app_->config );
    drag_detector_.SetConfiguration( app_->config );
    pinch_detector_.SetConfiguration( app_->config );
}

bool Engine::IsReady()
{
    if( has_focus_ )
        return true;

    return false;
}

void Engine::TransformPosition( ndk_helper::Vec2& vec )
{
    vec = ndk_helper::Vec2( 2.0f, 2.0f ) * vec
            / ndk_helper::Vec2( gl_context_->GetScreenWidth(), gl_context_->GetScreenHeight() )
            - ndk_helper::Vec2( 1.f, 1.f );
}

void Engine::ShowUI()
{
    JNIEnv *jni;
    app_->activity->vm->AttachCurrentThread( &jni, NULL );

    //Default class retrieval
    jclass clazz = jni->GetObjectClass( app_->activity->clazz );
    jmethodID methodID = jni->GetMethodID( clazz, "showUI", "()V" );
    jni->CallVoidMethod( app_->activity->clazz, methodID );

    app_->activity->vm->DetachCurrentThread();
    return;
}

void Engine::UpdateFPS( float fps )
{
    JNIEnv *jni;
    app_->activity->vm->AttachCurrentThread( &jni, NULL );

    //Default class retrieval
    jclass clazz = jni->GetObjectClass( app_->activity->clazz );
    jmethodID methodID = jni->GetMethodID( clazz, "updateFPS", "(F)V" );
    jni->CallVoidMethod( app_->activity->clazz, methodID, fps );

    app_->activity->vm->DetachCurrentThread();
    return;
}

Engine g_engine;

/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 */
void android_main( android_app* state )
{
    app_dummy();

    g_engine.SetState( state );

    //Init helper functions
    ndk_helper::JNIHelper::GetInstance()->Init( state->activity, HELPER_CLASS_NAME );

    state->userData = &g_engine;
    state->onAppCmd = Engine::HandleCmd;
    state->onInputEvent = Engine::HandleInput;

#ifdef USE_NDK_PROFILER
    monstartup("libMoreTeapotsNativeActivity.so");
#endif

    // Prepare to monitor accelerometer
    g_engine.InitSensors();

    // loop waiting for stuff to do.
    while( 1 )
    {
        // Read all pending events.
        int id;
        int events;
        android_poll_source* source;

        // If not animating, we will block forever waiting for events.
        // If animating, we loop until all events are read, then continue
        // to draw the next frame of animation.
        while( (id = ALooper_pollAll( g_engine.IsReady() ? 0 : -1, NULL, &events, (void**) &source ))
                >= 0 )
        {
            // Process this event.
            if( source != NULL )
                source->process( state, source );

            g_engine.ProcessSensors( id );

            // Check if we are exiting.
            if( state->destroyRequested != 0 )
            {
                g_engine.TermDisplay();
                return;
            }
        }

        if( g_engine.IsReady() )
        {
            // Drawing is throttled to the screen update rate, so there
            // is no need to do timing here.
            g_engine.DrawFrame();
        }
    }
}

