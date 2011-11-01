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
 */

package com.example.nativemedia;

import android.app.Activity;
import android.graphics.SurfaceTexture;
import android.media.MediaPlayer;
import android.os.Bundle;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.Spinner;
import java.io.IOException;

public class NativeMedia extends Activity {
    static final String TAG = "NativeMedia";

    String mSourceString = null;
    String mSinkString = null;

    // member variables for Java media player
    MediaPlayer mMediaPlayer;
    boolean mMediaPlayerIsPrepared = false;
    SurfaceView mSurfaceView1;
    SurfaceHolder mSurfaceHolder1;

    // member variables for native media player
    boolean mIsPlayingStreaming = false;
    SurfaceView mSurfaceView2;
    SurfaceHolder mSurfaceHolder2;

    VideoSink mSelectedVideoSink;
    VideoSink mJavaMediaPlayerVideoSink;
    VideoSink mNativeMediaPlayerVideoSink;

    SurfaceHolderVideoSink mSurfaceHolder1VideoSink, mSurfaceHolder2VideoSink;
    GLViewVideoSink mGLView1VideoSink, mGLView2VideoSink;

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        setContentView(R.layout.main);

        mGLView1 = (MyGLSurfaceView) findViewById(R.id.glsurfaceview1);
        mGLView2 = (MyGLSurfaceView) findViewById(R.id.glsurfaceview2);

        // initialize native media system
        createEngine();

        // set up the Surface 1 video sink
        mSurfaceView1 = (SurfaceView) findViewById(R.id.surfaceview1);
        mSurfaceHolder1 = mSurfaceView1.getHolder();

        mSurfaceHolder1.addCallback(new SurfaceHolder.Callback() {

            public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
                Log.v(TAG, "surfaceChanged format=" + format + ", width=" + width + ", height="
                        + height);
            }

            public void surfaceCreated(SurfaceHolder holder) {
                Log.v(TAG, "surfaceCreated");
                setSurface(holder.getSurface());
            }

            public void surfaceDestroyed(SurfaceHolder holder) {
                Log.v(TAG, "surfaceDestroyed");
            }

        });

        // set up the Surface 2 video sink
        mSurfaceView2 = (SurfaceView) findViewById(R.id.surfaceview2);
        mSurfaceHolder2 = mSurfaceView2.getHolder();

        mSurfaceHolder2.addCallback(new SurfaceHolder.Callback() {

            public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
                Log.v(TAG, "surfaceChanged format=" + format + ", width=" + width + ", height="
                        + height);
            }

            public void surfaceCreated(SurfaceHolder holder) {
                Log.v(TAG, "surfaceCreated");
                setSurface(holder.getSurface());
            }

            public void surfaceDestroyed(SurfaceHolder holder) {
                Log.v(TAG, "surfaceDestroyed");
            }

        });

        // create Java media player
        mMediaPlayer = new MediaPlayer();

        // set up Java media player listeners
        mMediaPlayer.setOnPreparedListener(new MediaPlayer.OnPreparedListener() {

            public void onPrepared(MediaPlayer mediaPlayer) {
                int width = mediaPlayer.getVideoWidth();
                int height = mediaPlayer.getVideoHeight();
                Log.v(TAG, "onPrepared width=" + width + ", height=" + height);
                if (width != 0 && height != 0 && mJavaMediaPlayerVideoSink != null) {
                    mJavaMediaPlayerVideoSink.setFixedSize(width, height);
                }
                mMediaPlayerIsPrepared = true;
                mediaPlayer.start();
            }

        });

        mMediaPlayer.setOnVideoSizeChangedListener(new MediaPlayer.OnVideoSizeChangedListener() {

            public void onVideoSizeChanged(MediaPlayer mediaPlayer, int width, int height) {
                Log.v(TAG, "onVideoSizeChanged width=" + width + ", height=" + height);
                if (width != 0 && height != 0 && mJavaMediaPlayerVideoSink != null) {
                    mJavaMediaPlayerVideoSink.setFixedSize(width, height);
                }
            }

        });

        // initialize content source spinner
        Spinner sourceSpinner = (Spinner) findViewById(R.id.source_spinner);
        ArrayAdapter<CharSequence> sourceAdapter = ArrayAdapter.createFromResource(
                this, R.array.source_array, android.R.layout.simple_spinner_item);
        sourceAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        sourceSpinner.setAdapter(sourceAdapter);
        sourceSpinner.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {

            public void onItemSelected(AdapterView<?> parent, View view, int pos, long id) {
                mSourceString = parent.getItemAtPosition(pos).toString();
                Log.v(TAG, "onItemSelected " + mSourceString);
            }

            public void onNothingSelected(AdapterView parent) {
                Log.v(TAG, "onNothingSelected");
                mSourceString = null;
            }

        });

        // initialize video sink spinner
        Spinner sinkSpinner = (Spinner) findViewById(R.id.sink_spinner);
        ArrayAdapter<CharSequence> sinkAdapter = ArrayAdapter.createFromResource(
                this, R.array.sink_array, android.R.layout.simple_spinner_item);
        sinkAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        sinkSpinner.setAdapter(sinkAdapter);
        sinkSpinner.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {

            public void onItemSelected(AdapterView<?> parent, View view, int pos, long id) {
                mSinkString = parent.getItemAtPosition(pos).toString();
                Log.v(TAG, "onItemSelected " + mSinkString);
                if ("Surface 1".equals(mSinkString)) {
                    if (mSurfaceHolder1VideoSink == null) {
                        mSurfaceHolder1VideoSink = new SurfaceHolderVideoSink(mSurfaceHolder1);
                    }
                    mSelectedVideoSink = mSurfaceHolder1VideoSink;
                } else if ("Surface 2".equals(mSinkString)) {
                    if (mSurfaceHolder2VideoSink == null) {
                        mSurfaceHolder2VideoSink = new SurfaceHolderVideoSink(mSurfaceHolder2);
                    }
                    mSelectedVideoSink = mSurfaceHolder2VideoSink;
                } else if ("SurfaceTexture 1".equals(mSinkString)) {
                    if (mGLView1VideoSink == null) {
                        mGLView1VideoSink = new GLViewVideoSink(mGLView1);
                    }
                    mSelectedVideoSink = mGLView1VideoSink;
                } else if ("SurfaceTexture 2".equals(mSinkString)) {
                    if (mGLView2VideoSink == null) {
                        mGLView2VideoSink = new GLViewVideoSink(mGLView2);
                    }
                    mSelectedVideoSink = mGLView2VideoSink;
                }
            }

            public void onNothingSelected(AdapterView parent) {
                Log.v(TAG, "onNothingSelected");
                mSinkString = null;
                mSelectedVideoSink = null;
            }

        });

        // initialize button click handlers

        // Java MediaPlayer start/pause

        ((Button) findViewById(R.id.start_java)).setOnClickListener(new View.OnClickListener() {

            public void onClick(View view) {
                if (mJavaMediaPlayerVideoSink == null) {
                    if (mSelectedVideoSink == null) {
                        return;
                    }
                    mSelectedVideoSink.useAsSinkForJava(mMediaPlayer);
                    mJavaMediaPlayerVideoSink = mSelectedVideoSink;
                }
                if (!mMediaPlayerIsPrepared) {
                    if (mSourceString != null) {
                        try {
                            mMediaPlayer.setDataSource(mSourceString);
                        } catch (IOException e) {
                            Log.e(TAG, "IOException " + e);
                        }
                        mMediaPlayer.prepareAsync();
                    }
                } else if (mMediaPlayer.isPlaying()) {
                    mMediaPlayer.pause();
                } else {
                    mMediaPlayer.start();
                }
            }

        });

        // native MediaPlayer start/pause

        ((Button) findViewById(R.id.start_native)).setOnClickListener(new View.OnClickListener() {

            boolean created = false;
            public void onClick(View view) {
                if (!created) {
                    if (mNativeMediaPlayerVideoSink == null) {
                        if (mSelectedVideoSink == null) {
                            return;
                        }
                        mSelectedVideoSink.useAsSinkForNative();
                        mNativeMediaPlayerVideoSink = mSelectedVideoSink;
                    }
                    if (mSourceString != null) {
                        created = createStreamingMediaPlayer(mSourceString);
                    }
                }
                if (created) {
                    mIsPlayingStreaming = !mIsPlayingStreaming;
                    setPlayingStreamingMediaPlayer(mIsPlayingStreaming);
                }
            }

        });

        // finish

        ((Button) findViewById(R.id.finish)).setOnClickListener(new View.OnClickListener() {

            public void onClick(View view) {
                finish();
            }

        });

        // Java MediaPlayer rewind

        ((Button) findViewById(R.id.rewind_java)).setOnClickListener(new View.OnClickListener() {

            public void onClick(View view) {
                if (mMediaPlayerIsPrepared) {
                    mMediaPlayer.seekTo(0);
                }
            }

        });

        // native MediaPlayer rewind

        ((Button) findViewById(R.id.rewind_native)).setOnClickListener(new View.OnClickListener() {

            public void onClick(View view) {
                if (mNativeMediaPlayerVideoSink != null) {
                    rewindStreamingMediaPlayer();
                }
            }

        });

    }

    /** Called when the activity is about to be paused. */
    @Override
    protected void onPause()
    {
        mIsPlayingStreaming = false;
        setPlayingStreamingMediaPlayer(false);
        mGLView1.onPause();
        mGLView2.onPause();
        super.onPause();
    }

    @Override
    protected void onResume() {
        super.onResume();
        mGLView1.onResume();
        mGLView2.onResume();
    }

    /** Called when the activity is about to be destroyed. */
    @Override
    protected void onDestroy()
    {
        shutdown();
        super.onDestroy();
    }

    private MyGLSurfaceView mGLView1, mGLView2;

    /** Native methods, implemented in jni folder */
    public static native void createEngine();
    public static native boolean createStreamingMediaPlayer(String filename);
    public static native void setPlayingStreamingMediaPlayer(boolean isPlaying);
    public static native void shutdown();
    public static native void setSurface(Surface surface);
    public static native void rewindStreamingMediaPlayer();

    /** Load jni .so on initialization */
    static {
         System.loadLibrary("native-media-jni");
    }

    // VideoSink abstracts out the difference between Surface and SurfaceTexture
    // aka SurfaceHolder and GLSurfaceView
    static abstract class VideoSink {

        abstract void setFixedSize(int width, int height);
        abstract void useAsSinkForJava(MediaPlayer mediaPlayer);
        abstract void useAsSinkForNative();

    }

    static class SurfaceHolderVideoSink extends VideoSink {

        private final SurfaceHolder mSurfaceHolder;

        SurfaceHolderVideoSink(SurfaceHolder surfaceHolder) {
            mSurfaceHolder = surfaceHolder;
        }

        void setFixedSize(int width, int height) {
            mSurfaceHolder.setFixedSize(width, height);
        }

        void useAsSinkForJava(MediaPlayer mediaPlayer) {
            // Use the newer MediaPlayer.setSurface(Surface) since API level 14
            // instead of MediaPlayer.setDisplay(mSurfaceHolder) since API level 1,
            // because setSurface also works with a Surface derived from a SurfaceTexture.
            Surface s = mSurfaceHolder.getSurface();
            mediaPlayer.setSurface(s);
            s.release();
        }

        void useAsSinkForNative() {
            Surface s = mSurfaceHolder.getSurface();
            setSurface(s);
            s.release();
        }

    }

    static class GLViewVideoSink extends VideoSink {

        private final MyGLSurfaceView mMyGLSurfaceView;

        GLViewVideoSink(MyGLSurfaceView myGLSurfaceView) {
            mMyGLSurfaceView = myGLSurfaceView;
        }

        void setFixedSize(int width, int height) {
        }

        void useAsSinkForJava(MediaPlayer mediaPlayer) {
            SurfaceTexture st = mMyGLSurfaceView.getSurfaceTexture();
            Surface s = new Surface(st);
            mediaPlayer.setSurface(s);
            s.release();
        }

        void useAsSinkForNative() {
            SurfaceTexture st = mMyGLSurfaceView.getSurfaceTexture();
            Surface s = new Surface(st);
            setSurface(s);
            s.release();
        }

    }

}
