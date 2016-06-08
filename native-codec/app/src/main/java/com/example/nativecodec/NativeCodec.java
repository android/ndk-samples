/*
 * Copyright (C) 2014 The Android Open Source Project
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

package com.example.nativecodec;

import android.app.Activity;
import android.content.res.AssetManager;
import android.graphics.SurfaceTexture;
import android.os.Bundle;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.RadioButton;
import android.widget.Spinner;

import java.io.IOException;

public class NativeCodec extends Activity {
    static final String TAG = "NativeCodec";

    String mSourceString = null;

    SurfaceView mSurfaceView1;
    SurfaceHolder mSurfaceHolder1;

    VideoSink mSelectedVideoSink;
    VideoSink mNativeCodecPlayerVideoSink;

    SurfaceHolderVideoSink mSurfaceHolder1VideoSink;
    GLViewVideoSink mGLView1VideoSink;

    boolean mCreated = false;
    boolean mIsPlaying = false;

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        setContentView(R.layout.main);

        mGLView1 = (MyGLSurfaceView) findViewById(R.id.glsurfaceview1);

        // set up the Surface 1 video sink
        mSurfaceView1 = (SurfaceView) findViewById(R.id.surfaceview1);
        mSurfaceHolder1 = mSurfaceView1.getHolder();

        mSurfaceHolder1.addCallback(new SurfaceHolder.Callback() {

            @Override
            public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
                Log.v(TAG, "surfaceChanged format=" + format + ", width=" + width + ", height="
                        + height);
            }

            @Override
            public void surfaceCreated(SurfaceHolder holder) {
                Log.v(TAG, "surfaceCreated");
                if (mRadio1.isChecked()) {
                    setSurface(holder.getSurface());
                }
            }

            @Override
            public void surfaceDestroyed(SurfaceHolder holder) {
                Log.v(TAG, "surfaceDestroyed");
            }

        });

        // initialize content source spinner
        Spinner sourceSpinner = (Spinner) findViewById(R.id.source_spinner);
        ArrayAdapter<CharSequence> sourceAdapter = ArrayAdapter.createFromResource(
                this, R.array.source_array, android.R.layout.simple_spinner_item);
        sourceAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        sourceSpinner.setAdapter(sourceAdapter);
        sourceSpinner.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {

            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int pos, long id) {
                mSourceString = parent.getItemAtPosition(pos).toString();
                Log.v(TAG, "onItemSelected " + mSourceString);
            }

            @Override
            public void onNothingSelected(AdapterView parent) {
                Log.v(TAG, "onNothingSelected");
                mSourceString = null;
            }

        });

        mRadio1 = (RadioButton) findViewById(R.id.radio1);
        mRadio2 = (RadioButton) findViewById(R.id.radio2);

        OnCheckedChangeListener checklistener = new CompoundButton.OnCheckedChangeListener() {

          @Override
          public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
              Log.i("@@@@", "oncheckedchanged");
              if (buttonView == mRadio1 && isChecked) {
                  mRadio2.setChecked(false);
              }
              if (buttonView == mRadio2 && isChecked) {
                  mRadio1.setChecked(false);
              }
              if (isChecked) {
                  if (mRadio1.isChecked()) {
                      if (mSurfaceHolder1VideoSink == null) {
                          mSurfaceHolder1VideoSink = new SurfaceHolderVideoSink(mSurfaceHolder1);
                      }
                      mSelectedVideoSink = mSurfaceHolder1VideoSink;
                      mGLView1.onPause();
                      Log.i("@@@@", "glview pause");
                  } else {
                      mGLView1.onResume();
                      if (mGLView1VideoSink == null) {
                          mGLView1VideoSink = new GLViewVideoSink(mGLView1);
                      }
                      mSelectedVideoSink = mGLView1VideoSink;
                  }
                  switchSurface();
              }
          }
        };
        mRadio1.setOnCheckedChangeListener(checklistener);
        mRadio2.setOnCheckedChangeListener(checklistener);
        mRadio2.toggle();

        // the surfaces themselves are easier targets than the radio buttons
        mSurfaceView1.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mRadio1.toggle();
            }
        });
        mGLView1.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mRadio2.toggle();
            }
      });

        // initialize button click handlers

        // native MediaPlayer start/pause
        ((Button) findViewById(R.id.start_native)).setOnClickListener(new View.OnClickListener() {

            @Override
            public void onClick(View view) {
                if (!mCreated) {
                    if (mNativeCodecPlayerVideoSink == null) {
                        if (mSelectedVideoSink == null) {
                            return;
                        }
                        mSelectedVideoSink.useAsSinkForNative();
                        mNativeCodecPlayerVideoSink = mSelectedVideoSink;
                    }
                    if (mSourceString != null) {
                        mCreated = createStreamingMediaPlayer(getResources().getAssets(),
                                mSourceString);
                    }
                }
                if (mCreated) {
                    mIsPlaying = !mIsPlaying;
                    setPlayingStreamingMediaPlayer(mIsPlaying);
                }
            }

        });


        // native MediaPlayer rewind
        ((Button) findViewById(R.id.rewind_native)).setOnClickListener(new View.OnClickListener() {

            @Override
            public void onClick(View view) {
                if (mNativeCodecPlayerVideoSink != null) {
                    rewindStreamingMediaPlayer();
                }
            }

        });
    }

    void switchSurface() {
        if (mCreated && mNativeCodecPlayerVideoSink != mSelectedVideoSink) {
            // shutdown and recreate on other surface
          Log.i("@@@", "shutting down player");
            shutdown();
            mCreated = false;
            mSelectedVideoSink.useAsSinkForNative();
            mNativeCodecPlayerVideoSink = mSelectedVideoSink;
            if (mSourceString != null) {
                Log.i("@@@", "recreating player");
                mCreated = createStreamingMediaPlayer(getResources().getAssets(),mSourceString);
                mIsPlaying = false;
            }
        }
    }

    /** Called when the activity is about to be paused. */
    @Override
    protected void onPause()
    {
        mIsPlaying = false;
        setPlayingStreamingMediaPlayer(false);
        mGLView1.onPause();
        super.onPause();
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (mRadio2.isChecked()) {
            mGLView1.onResume();
        }
    }

    /** Called when the activity is about to be destroyed. */
    @Override
    protected void onDestroy()
    {
        shutdown();
        mCreated = false;
        super.onDestroy();
    }

    private MyGLSurfaceView mGLView1;

    private RadioButton mRadio1;

    private RadioButton mRadio2;

    /** Native methods, implemented in jni folder */
    public static native void createEngine();
    public static native boolean createStreamingMediaPlayer(AssetManager assetMgr, String filename);
    public static native void setPlayingStreamingMediaPlayer(boolean isPlaying);
    public static native void shutdown();
    public static native void setSurface(Surface surface);
    public static native void rewindStreamingMediaPlayer();

    /** Load jni .so on initialization */
    static {
         System.loadLibrary("native-codec-jni");
    }

    // VideoSink abstracts out the difference between Surface and SurfaceTexture
    // aka SurfaceHolder and GLSurfaceView
    static abstract class VideoSink {

        abstract void setFixedSize(int width, int height);
        abstract void useAsSinkForNative();

    }

    static class SurfaceHolderVideoSink extends VideoSink {

        private final SurfaceHolder mSurfaceHolder;

        SurfaceHolderVideoSink(SurfaceHolder surfaceHolder) {
            mSurfaceHolder = surfaceHolder;
        }

        @Override
        void setFixedSize(int width, int height) {
            mSurfaceHolder.setFixedSize(width, height);
        }

        @Override
        void useAsSinkForNative() {
            Surface s = mSurfaceHolder.getSurface();
            Log.i("@@@", "setting surface " + s);
            setSurface(s);
        }

    }

    static class GLViewVideoSink extends VideoSink {

        private final MyGLSurfaceView mMyGLSurfaceView;

        GLViewVideoSink(MyGLSurfaceView myGLSurfaceView) {
            mMyGLSurfaceView = myGLSurfaceView;
        }

        @Override
        void setFixedSize(int width, int height) {
        }

        @Override
        void useAsSinkForNative() {
            SurfaceTexture st = mMyGLSurfaceView.getSurfaceTexture();
            Surface s = new Surface(st);
            setSurface(s);
            s.release();
        }

    }

}
