/*
 * Copyright 2015 The Android Open Source Project
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

package com.google.sample.echo;

import android.app.Activity;
import android.content.Context;
import android.media.AudioManager;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.TextView;

public class MainActivity extends Activity {
    public static final String AUDIO_SAMPLE = "AUDIO_SAMPLE:";
    TextView status_view;
    String  nativeSampleRate;
    String  nativeSampleBufSize;
    String  nativeSampleFormat;
    Boolean isPlaying;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        status_view = (TextView)findViewById(R.id.statusView);
        queryNativeAudioParameters();

        // initialize native audio system
        updateNativeAudioUI();
        createSLEngine(Integer.parseInt(nativeSampleRate), Integer.parseInt(nativeSampleBufSize));
        isPlaying = false;
    }
    @Override
    protected void onDestroy() {
        if( isPlaying ) {
            stopPlay();
        }
        //shutdown();
        deleteSLEngine();
        isPlaying = false;
        super.onDestroy();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
            return true;
        }

        return super.onOptionsItemSelected(item);
    }

    public void startEcho(View view) {
        status_view.setText("StartCapture Button Clicked\n");
        if(isPlaying) {
            return;
        }
        if(!createSLBufferQueueAudioPlayer()) {
            status_view.setText("Failed to create Audio Player");
            return;
        }
        if(!createAudioRecorder()) {
            deleteSLBufferQueueAudioPlayer();
            status_view.setText("Failed to create Audio Recorder");
            return;
        }
        startPlay();   //this must include startRecording()
        isPlaying = true;
        status_view.setText("Engine Echoing ....");
    }

    public void stopEcho(View view) {
        if(!isPlaying) {
            return;
        }
        stopPlay();  //this must include stopRecording()
        updateNativeAudioUI();
        deleteSLBufferQueueAudioPlayer();
        deleteAudioRecorder();
        isPlaying = false;
    }
    public void getLowLatencyParameters(View view) {
        updateNativeAudioUI();
        return;
    }

    private void queryNativeAudioParameters() {
        AudioManager myAudioMgr = (AudioManager) getSystemService(Context.AUDIO_SERVICE);
        nativeSampleRate  =  myAudioMgr.getProperty(AudioManager.PROPERTY_OUTPUT_SAMPLE_RATE);
        nativeSampleBufSize =myAudioMgr.getProperty(AudioManager.PROPERTY_OUTPUT_FRAMES_PER_BUFFER);
        nativeSampleFormat ="";    //TODO: find a way to get the native audio format
    }
    private void updateNativeAudioUI() {
        status_view.setText("nativeSampleRate    = " + nativeSampleRate + "\n" +
                "nativeSampleBufSize = " + nativeSampleBufSize + "\n" +
                "nativeSampleFormat  = " + nativeSampleFormat);

    }
    /*
     * Loading our Libs
     */
    static {
        System.loadLibrary("echo");
    }

    /*
     * jni function implementations...
     */
    public static native void createSLEngine(int rate, int framesPerBuf);
    public static native void deleteSLEngine();

    public static native boolean createSLBufferQueueAudioPlayer();
    public static native void deleteSLBufferQueueAudioPlayer();

    public static native boolean createAudioRecorder();
    public static native void deleteAudioRecorder();
    public static native void startPlay();
    public static native void stopPlay();
}
