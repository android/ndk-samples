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

#include <android_native_app_glue.h>

#include <errno.h>
#include <jni.h>
#include <sys/time.h>
#include <time.h>
#include <android/log.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define  LOG_TAG    "libplasma"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGW(...)  __android_log_print(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

/* Set to 1 to enable debug log traces. */
#define DEBUG 0

/* Set to 1 to optimize memory stores when generating plasma. */
#define OPTIMIZE_WRITES  1

/* Return current time in milliseconds */
static double now_ms(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec*1000. + tv.tv_usec/1000.;
}

/* We're going to perform computations for every pixel of the target
 * bitmap. floating-point operations are very slow on ARMv5, and not
 * too bad on ARMv7 with the exception of trigonometric functions.
 *
 * For better performance on all platforms, we're going to use fixed-point
 * arithmetic and all kinds of tricks
 */

typedef int32_t  Fixed;

#define  FIXED_BITS           16
#define  FIXED_ONE            (1 << FIXED_BITS)
#define  FIXED_AVERAGE(x,y)   (((x) + (y)) >> 1)

#define  FIXED_FROM_INT(x)    ((x) << FIXED_BITS)
#define  FIXED_TO_INT(x)      ((x) >> FIXED_BITS)

#define  FIXED_FROM_FLOAT(x)  ((Fixed)((x)*FIXED_ONE))
#define  FIXED_TO_FLOAT(x)    ((x)/(1.*FIXED_ONE))

#define  FIXED_MUL(x,y)       (((int64_t)(x) * (y)) >> FIXED_BITS)
#define  FIXED_DIV(x,y)       (((int64_t)(x) * FIXED_ONE) / (y))

#define  FIXED_DIV2(x)        ((x) >> 1)
#define  FIXED_AVERAGE(x,y)   (((x) + (y)) >> 1)

#define  FIXED_FRAC(x)        ((x) & ((1 << FIXED_BITS)-1))
#define  FIXED_TRUNC(x)       ((x) & ~((1 << FIXED_BITS)-1))

#define  FIXED_FROM_INT_FLOAT(x,f)   (Fixed)((x)*(FIXED_ONE*(f)))

typedef int32_t  Angle;

#define  ANGLE_BITS              9

#if ANGLE_BITS < 8
#  error ANGLE_BITS must be at least 8
#endif

#define  ANGLE_2PI               (1 << ANGLE_BITS)
#define  ANGLE_PI                (1 << (ANGLE_BITS-1))
#define  ANGLE_PI2               (1 << (ANGLE_BITS-2))
#define  ANGLE_PI4               (1 << (ANGLE_BITS-3))

#define  ANGLE_FROM_FLOAT(x)   (Angle)((x)*ANGLE_PI/M_PI)
#define  ANGLE_TO_FLOAT(x)     ((x)*M_PI/ANGLE_PI)

#if ANGLE_BITS <= FIXED_BITS
#  define  ANGLE_FROM_FIXED(x)     (Angle)((x) >> (FIXED_BITS - ANGLE_BITS))
#  define  ANGLE_TO_FIXED(x)       (Fixed)((x) << (FIXED_BITS - ANGLE_BITS))
#else
#  define  ANGLE_FROM_FIXED(x)     (Angle)((x) << (ANGLE_BITS - FIXED_BITS))
#  define  ANGLE_TO_FIXED(x)       (Fixed)((x) >> (ANGLE_BITS - FIXED_BITS))
#endif

static Fixed  angle_sin_tab[ANGLE_2PI+1];

static void init_angles(void)
{
    int  nn;
    for (nn = 0; nn < ANGLE_2PI+1; nn++) {
        double  radians = nn*M_PI/ANGLE_PI;
        angle_sin_tab[nn] = FIXED_FROM_FLOAT(sin(radians));
    }
}

static __inline__ Fixed angle_sin( Angle  a )
{
    return angle_sin_tab[(uint32_t)a & (ANGLE_2PI-1)];
}

static __inline__ Fixed angle_cos( Angle  a )
{
    return angle_sin(a + ANGLE_PI2);
}

static __inline__ Fixed fixed_sin( Fixed  f )
{
    return angle_sin(ANGLE_FROM_FIXED(f));
}

static __inline__ Fixed  fixed_cos( Fixed  f )
{
    return angle_cos(ANGLE_FROM_FIXED(f));
}

/* Color palette used for rendering the plasma */
#define  PALETTE_BITS   8
#define  PALETTE_SIZE   (1 << PALETTE_BITS)

#if PALETTE_BITS > FIXED_BITS
#  error PALETTE_BITS must be smaller than FIXED_BITS 
#endif

static uint16_t  palette[PALETTE_SIZE];

static uint16_t  make565(int red, int green, int blue)
{
    return (uint16_t)( ((red   << 8) & 0xf800) |
                       ((green << 2) & 0x03e0) |
                       ((blue  >> 3) & 0x001f) );
}

static void init_palette(void)
{
    int  nn, mm = 0;
    /* fun with colors */
    for (nn = 0; nn < PALETTE_SIZE/4; nn++) {
        int  jj = (nn-mm)*4*255/PALETTE_SIZE;
        palette[nn] = make565(255, jj, 255-jj);
    }

    for ( mm = nn; nn < PALETTE_SIZE/2; nn++ ) {
        int  jj = (nn-mm)*4*255/PALETTE_SIZE;
        palette[nn] = make565(255-jj, 255, jj);
    }

    for ( mm = nn; nn < PALETTE_SIZE*3/4; nn++ ) {
        int  jj = (nn-mm)*4*255/PALETTE_SIZE;
        palette[nn] = make565(0, 255-jj, 255);
    }

    for ( mm = nn; nn < PALETTE_SIZE; nn++ ) {
        int  jj = (nn-mm)*4*255/PALETTE_SIZE;
        palette[nn] = make565(jj, 0, 255);
    }
}

static __inline__ uint16_t  palette_from_fixed( Fixed  x )
{
    if (x < 0) x = -x;
    if (x >= FIXED_ONE) x = FIXED_ONE-1;
    int  idx = FIXED_FRAC(x) >> (FIXED_BITS - PALETTE_BITS);
    return palette[idx & (PALETTE_SIZE-1)];
}

/* Angles expressed as fixed point radians */

static void init_tables(void)
{
    init_palette();
    init_angles();
}

static void fill_plasma(ANativeWindow_Buffer* buffer, double  t)
{
    Fixed yt1 = FIXED_FROM_FLOAT(t/1230.);
    Fixed yt2 = yt1;
    Fixed xt10 = FIXED_FROM_FLOAT(t/3000.);
    Fixed xt20 = xt10;

#define  YT1_INCR   FIXED_FROM_FLOAT(1/100.)
#define  YT2_INCR   FIXED_FROM_FLOAT(1/163.)

    void* pixels = buffer->bits;
    //LOGI("width=%d height=%d stride=%d format=%d", buffer->width, buffer->height,
    //        buffer->stride, buffer->format);

    int  yy;
    for (yy = 0; yy < buffer->height; yy++) {
        uint16_t*  line = (uint16_t*)pixels;
        Fixed      base = fixed_sin(yt1) + fixed_sin(yt2);
        Fixed      xt1 = xt10;
        Fixed      xt2 = xt20;

        yt1 += YT1_INCR;
        yt2 += YT2_INCR;

#define  XT1_INCR  FIXED_FROM_FLOAT(1/173.)
#define  XT2_INCR  FIXED_FROM_FLOAT(1/242.)

#if OPTIMIZE_WRITES
        /* optimize memory writes by generating one aligned 32-bit store
         * for every pair of pixels.
         */
        uint16_t*  line_end = line + buffer->width;

        if (line < line_end) {
            if (((uint32_t)(uintptr_t)line & 3) != 0) {
                Fixed ii = base + fixed_sin(xt1) + fixed_sin(xt2);

                xt1 += XT1_INCR;
                xt2 += XT2_INCR;

                line[0] = palette_from_fixed(ii >> 2);
                line++;
            }

            while (line + 2 <= line_end) {
                Fixed i1 = base + fixed_sin(xt1) + fixed_sin(xt2);
                xt1 += XT1_INCR;
                xt2 += XT2_INCR;

                Fixed i2 = base + fixed_sin(xt1) + fixed_sin(xt2);
                xt1 += XT1_INCR;
                xt2 += XT2_INCR;

                uint32_t  pixel = ((uint32_t)palette_from_fixed(i1 >> 2) << 16) |
                                   (uint32_t)palette_from_fixed(i2 >> 2);

                ((uint32_t*)line)[0] = pixel;
                line += 2;
            }

            if (line < line_end) {
                Fixed ii = base + fixed_sin(xt1) + fixed_sin(xt2);
                line[0] = palette_from_fixed(ii >> 2);
                line++;
            }
        }
#else /* !OPTIMIZE_WRITES */
        int xx;
        for (xx = 0; xx < buffer->width; xx++) {

            Fixed ii = base + fixed_sin(xt1) + fixed_sin(xt2);

            xt1 += XT1_INCR;
            xt2 += XT2_INCR;

            line[xx] = palette_from_fixed(ii / 4);
        }
#endif /* !OPTIMIZE_WRITES */

        // go to next line
        pixels = (uint16_t*)pixels + buffer->stride;
    }
}

/* simple stats management */
typedef struct {
    double  renderTime;
    double  frameTime;
} FrameStats;

#define  MAX_FRAME_STATS  200
#define  MAX_PERIOD_MS    1500

typedef struct {
    double  firstTime;
    double  lastTime;
    double  frameTime;

    int         firstFrame;
    int         numFrames;
    FrameStats  frames[ MAX_FRAME_STATS ];
} Stats;

static void
stats_init( Stats*  s )
{
    s->lastTime = now_ms();
    s->firstTime = 0.;
    s->firstFrame = 0;
    s->numFrames  = 0;
}

static void
stats_startFrame( Stats*  s )
{
    s->frameTime = now_ms();
}

static void
stats_endFrame( Stats*  s )
{
    double now = now_ms();
    double renderTime = now - s->frameTime;
    double frameTime  = now - s->lastTime;
    int nn;

    if (now - s->firstTime >= MAX_PERIOD_MS) {
        if (s->numFrames > 0) {
            double minRender, maxRender, avgRender;
            double minFrame, maxFrame, avgFrame;
            int count;

            nn = s->firstFrame;
            minRender = maxRender = avgRender = s->frames[nn].renderTime;
            minFrame  = maxFrame  = avgFrame  = s->frames[nn].frameTime;
            for (count = s->numFrames; count > 0; count-- ) {
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
                avgFrame  += frame;
            }
            avgRender /= s->numFrames;
            avgFrame  /= s->numFrames;

            LOGI("frame/s (avg,min,max) = (%.1f,%.1f,%.1f) "
                 "render time ms (avg,min,max) = (%.1f,%.1f,%.1f)\n",
                 1000./avgFrame, 1000./maxFrame, 1000./minFrame,
                 avgRender, minRender, maxRender);
        }
        s->numFrames  = 0;
        s->firstFrame = 0;
        s->firstTime  = now;
    }

    nn = s->firstFrame + s->numFrames;
    if (nn >= MAX_FRAME_STATS)
        nn -= MAX_FRAME_STATS;

    s->frames[nn].renderTime = renderTime;
    s->frames[nn].frameTime  = frameTime;

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
    struct android_app* app;

    Stats stats;

    int animating;
};

static int64_t start_ms;
static void engine_draw_frame(struct engine* engine) {
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
    int64_t time_ms = (((int64_t)now.tv_sec)*1000000000LL + now.tv_nsec)/1000000;
    time_ms -= start_ms;

    /* Now fill the values with a nice little plasma */
    fill_plasma(&buffer, time_ms);

    ANativeWindow_unlockAndPost(engine->app->window);

    stats_endFrame(&engine->stats);
}

static void engine_term_display(struct engine* engine) {
    engine->animating = 0;
}

static int32_t engine_handle_input(struct android_app* app, AInputEvent* event) {
    struct engine* engine = (struct engine*)app->userData;
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

static void engine_handle_cmd(struct android_app* app, int32_t cmd) {
    struct engine* engine = (struct engine*)app->userData;
    switch (cmd) {
        case APP_CMD_INIT_WINDOW:
            if (engine->app->window != NULL) {
                engine_draw_frame(engine);
            }
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

void android_main(struct android_app* state) {
    static int init;

    struct engine engine;

    // Make sure glue isn't stripped.
    app_dummy();

    memset(&engine, 0, sizeof(engine));
    state->userData = &engine;
    state->onAppCmd = engine_handle_cmd;
    state->onInputEvent = engine_handle_input;
    engine.app = state;

    if (!init) {
        init_tables();
        init = 1;
    }

    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    start_ms = (((int64_t)now.tv_sec)*1000000000LL + now.tv_nsec)/1000000;

    stats_init(&engine.stats);

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

            // Check if we are exiting.
            if (state->destroyRequested != 0) {
                LOGI("Engine thread destroy requested!");
                engine_term_display(&engine);
                return;
            }
        }

        if (engine.animating) {
            engine_draw_frame(&engine);
        }
    }
}
