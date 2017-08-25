/*
 * Copyright (C) 2017 The Android Open Source Project
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

#include <android/sensor.h>
#include <android_native_app_glue.h>
#include "ImageViewEngine.h"

/**
 * Process the next main command.
 */
static void engine_handle_cmd(struct android_app* app, int32_t cmd) {
  ImageViewEngine* engine = (ImageViewEngine*)app->userData;
  switch (cmd) {
    case APP_CMD_SAVE_STATE:
      break;
    case APP_CMD_INIT_WINDOW:
      // The window is being shown, get it ready.
      if (app->window != NULL) {
        engine->InitializeDisplay();
        engine->DrawFrame();
        engine->EnableAnimation(true);
      }
      break;
    case APP_CMD_TERM_WINDOW:
      // The window is being hidden or closed, clean it up.
      engine->TerminateDisplay();
      break;
    case APP_CMD_GAINED_FOCUS:
      // When our app gains focus, we start monitoring the accelerometer.
      if (engine->accelerometerSensor != NULL) {
        ASensorEventQueue_enableSensor(engine->sensorEventQueue,
                                       engine->accelerometerSensor);
        // We'd like to get 60 events per second (in us).
        ASensorEventQueue_setEventRate(engine->sensorEventQueue,
                                       engine->accelerometerSensor,
                                       (1000L/60)*1000);
        engine->EnableAnimation(true);
      }
      break;
    case APP_CMD_LOST_FOCUS:
      // When our app loses focus, we stop monitoring the accelerometer.
      // This is to avoid consuming battery while not being used.
      if (engine->accelerometerSensor != NULL) {
        ASensorEventQueue_disableSensor(engine->sensorEventQueue,
                                        engine->accelerometerSensor);
      }
      // Also stop animating_.
      engine->EnableAnimation(false);
      engine->DrawFrame();
      break;
  }
}

/*
 * Process the next input event.
 */
static int32_t engine_handle_input(struct android_app* app, AInputEvent* event) {
  ImageViewEngine* engine = (ImageViewEngine*)app->userData;

  if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
    engine->ProcessInputEvent(event);

    return 1;
  }
  return 0;
}

/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 */
void android_main(struct android_app* state) {
    ImageViewEngine engine(state);

    state->userData = &engine;
    state->onAppCmd = engine_handle_cmd;
    state->onInputEvent = engine_handle_input;

    // Prepare to monitor accelerometer
    engine.sensorManager = ASensorManager_getInstanceForPackage("com.example.widecolor");
    engine.accelerometerSensor = ASensorManager_getDefaultSensor(
                                        engine.sensorManager,
                                        ASENSOR_TYPE_ACCELEROMETER);
    engine.sensorEventQueue = ASensorManager_createEventQueue(
                                    engine.sensorManager,
                                    state->looper, LOOPER_ID_USER,
                                    NULL, NULL);

    while (1) {
        // Read all pending events.
        int ident;
        int events;
        struct android_poll_source* source;

        // If not animating_, we will block forever waiting for events.
        // If animating_, we loop until all events are read, then continue
        // to draw the next frame of animation.
        while ((ident=ALooper_pollAll(engine.GetAnimationStatus() ? 0 : -1, NULL, &events,
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
                engine.TerminateDisplay();
                return;
            }
        }

        if (engine.GetAnimationStatus()) {
            engine.DrawFrame();
        }
    }
}


