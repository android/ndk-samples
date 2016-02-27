/*
 * Copyright (C) 2015 The Android Open Source Project
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
#include <android_native_app_glue.h>

#include <android/log.h>

#include <stdio.h>
#include "plasma_renderer.h"

#define  LOG_TAG    "libplasma"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGW(...)  __android_log_print(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

// Return current time in milliseconds
static double now_ms(void) {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec * 1000. + tv.tv_usec / 1000.;
}

// simple stats management
typedef struct {
  double renderTime;
  double frameTime;
} FrameStats;

#define  MAX_FRAME_STATS  200
#define  MAX_PERIOD_MS    1500

typedef struct {
  double firstTime;
  double lastTime;
  double frameTime;

  int firstFrame;
  int numFrames;
  FrameStats frames[MAX_FRAME_STATS];
} Stats;

static void stats_init(Stats *s) {
  s->lastTime = now_ms();
  s->firstTime = 0.;
  s->firstFrame = 0;
  s->numFrames = 0;
}

static void stats_startFrame(Stats *s) {
  s->frameTime = now_ms();
}

static void
stats_endFrame(Stats *s) {
  double now = now_ms();
  double renderTime = now - s->frameTime;
  double frameTime = now - s->lastTime;
  int nn;

  if (now - s->firstTime >= MAX_PERIOD_MS) {
    if (s->numFrames > 0) {
      double minRender, maxRender, avgRender;
      double minFrame, maxFrame, avgFrame;
      int count;

      nn = s->firstFrame;
      minRender = maxRender = avgRender = s->frames[nn].renderTime;
      minFrame = maxFrame = avgFrame = s->frames[nn].frameTime;
      for (count = s->numFrames; count > 0; count--) {
        nn += 1;
        if (nn >= MAX_FRAME_STATS)
          nn -= MAX_FRAME_STATS;
        double render = s->frames[nn].renderTime;
        if (render < minRender) minRender = render;
        if (render > maxRender) maxRender = render;
        double frame = s->frames[nn].frameTime;
        if (frame < minFrame) minFrame = frame;
        if (frame > maxFrame) maxFrame = frame;
        avgRender += render;
        avgFrame += frame;
      }
      avgRender /= s->numFrames;
      avgFrame /= s->numFrames;

      LOGI("frame/s (avg,min,max) = (%.1f,%.1f,%.1f) "
               "render time ms (avg,min,max) = (%.1f,%.1f,%.1f)\n",
           1000. / avgFrame, 1000. / maxFrame, 1000. / minFrame,
           avgRender, minRender, maxRender);
    }
    s->numFrames = 0;
    s->firstFrame = 0;
    s->firstTime = now;
  }

  nn = s->firstFrame + s->numFrames;
  if (nn >= MAX_FRAME_STATS)
    nn -= MAX_FRAME_STATS;

  s->frames[nn].renderTime = renderTime;
  s->frames[nn].frameTime = frameTime;

  if (s->numFrames < MAX_FRAME_STATS) {
    s->numFrames += 1;
  } else {
    s->firstFrame += 1;
    if (s->firstFrame >= MAX_FRAME_STATS)
      s->firstFrame -= MAX_FRAME_STATS;
  }

  s->lastTime = now;
}

// ----------------------------------------------------------------------

struct engine {
  struct android_app *app;
  PlasmaRenderer *plasmaRenderer;

  Stats stats;

  int initialised;
  int animating;
};

static int64_t start_ms;

static void engine_draw_frame(struct engine *engine) {
  if (engine->app->window == NULL) {
    // No window.
    return;
  }

  ANativeWindow_Buffer buffer;
  if (ANativeWindow_lock(engine->app->window, &buffer, NULL) < 0) {
    LOGW("Unable to lock window buffer");
    return;
  }

  stats_startFrame(&engine->stats);

  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  int64_t time_ms = (((int64_t) now.tv_sec) * 1000000000LL + now.tv_nsec) / 1000000;
  time_ms -= start_ms;

  /* Now fill the values with a nice little plasma */
  engine->plasmaRenderer->fill_plasma(&buffer, time_ms);

  ANativeWindow_unlockAndPost(engine->app->window);

  stats_endFrame(&engine->stats);
}

static void engine_init(struct engine *engine) {
  if (!engine->initialised) {
    int framebufferFormat = ANativeWindow_getFormat(engine->app->window);
    if (framebufferFormat == WINDOW_FORMAT_RGBA_8888) {
      engine->plasmaRenderer = new PlasmaRendererRGBA8888();
    }
    else if (framebufferFormat == WINDOW_FORMAT_RGB_565) {
      engine->plasmaRenderer = new PlasmaRendererRGB565();
    }
    engine->plasmaRenderer->init();
    engine->initialised = 1;
    engine->animating = 1;
  }
}

static void engine_term_display(struct engine *engine) {
  engine->animating = 0;
}

static int32_t engine_handle_input(struct android_app *app, AInputEvent *event) {
  struct engine *engine = (struct engine *) app->userData;
  if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
    engine->animating = 1;
    return 1;
  } else if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_KEY) {
    LOGI("Key event: action=%d keyCode=%d metaState=0x%x",
         AKeyEvent_getAction(event),
         AKeyEvent_getKeyCode(event),
         AKeyEvent_getMetaState(event));
  }

  return 0;
}

static void engine_handle_cmd(struct android_app *app, int32_t cmd) {
  struct engine *engine = (struct engine *) app->userData;
  switch (cmd) {
    case APP_CMD_INIT_WINDOW:
      engine_init(engine);
      break;
    case APP_CMD_TERM_WINDOW:
      engine_term_display(engine);
      break;
    case APP_CMD_LOST_FOCUS:
      engine->animating = 0;
      engine_draw_frame(engine);
      break;
  }
}

void android_main(struct android_app *state) {
  struct engine engine;

  // Make sure glue isn't stripped.
  app_dummy();

  memset(&engine, 0, sizeof(engine));
  state->userData = &engine;
  state->onAppCmd = engine_handle_cmd;
  state->onInputEvent = engine_handle_input;
  engine.app = state;

  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  start_ms = (((int64_t) now.tv_sec) * 1000000000LL + now.tv_nsec) / 1000000;

  stats_init(&engine.stats);

  // loop waiting for stuff to do.
  while (1) {
    // Read all pending events.
    int events;
    struct android_poll_source *source;

    // If not animating, we will block forever waiting for events.
    // If animating, we loop until all events are read, then continue
    // to draw the next frame of animation.
    while ((ALooper_pollAll(engine.animating ? 0 : -1, NULL, &events,
                            (void **) &source)) >= 0) {

      // Process this event.
      if (source != NULL)
        source->process(state, source);

      // Check if we are exiting.
      if (state->destroyRequested != 0) {
        LOGI("Engine thread destroy requested!");
        engine_term_display(&engine);
        return;
      }
    }

    if (engine.animating)
      engine_draw_frame(&engine);
  }
}
