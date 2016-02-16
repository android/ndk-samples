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
import android.app.FragmentTransaction;
import android.content.Context;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioRecord;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.TextView;

public class MainActivity extends Activity {
    public static final String AUDIO_SAMPLE = "AUDIO_SAMPLE:";
    private static final String PERMISSION_FRAGMENT_TAG = "sample.echo.permissionFragment";
    TextView status_view;
    String  nativeSampleRate;
    String  nativeSampleBufSize;
    boolean supportRecording;

    PermissionRequestFragment recordAudioFragment;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        status_view = (TextView)findViewById(R.id.statusView);
        queryNativeAudioParameters();

        // initialize native audio system
        updateNativeAudioUI();

        // add the permission checking fragment
        recordAudioFragment =
                (PermissionRequestFragment) getFragmentManager().findFragmentByTag(PERMISSION_FRAGMENT_TAG);
        if (recordAudioFragment == null) {
            recordAudioFragment = new PermissionRequestFragment();
            FragmentTransaction trans = getFragmentManager().beginTransaction();
            trans.add(recordAudioFragment, PERMISSION_FRAGMENT_TAG);
            trans.commit();
        }
        if (supportRecording) {
            createSLEngine(Integer.parseInt(nativeSampleRate), Integer.parseInt(nativeSampleBufSize));
        }
    }

    @Override
    protected void onDestroy() {
        if (supportRecording) {
            stopEchoProcessing();
            deleteSLEngine();
        }
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
       startEchoProcessing();
    }

    public void stopEcho(View view) {
        stopEchoProcessing();
        updateNativeAudioUI();
    }
    public void getLowLatencyParameters(View view) {
        updateNativeAudioUI();
        return;
    }

    private void queryNativeAudioParameters() {
        AudioManager myAudioMgr = (AudioManager) getSystemService(Context.AUDIO_SERVICE);
        nativeSampleRate  =  myAudioMgr.getProperty(AudioManager.PROPERTY_OUTPUT_SAMPLE_RATE);
        nativeSampleBufSize =myAudioMgr.getProperty(AudioManager.PROPERTY_OUTPUT_FRAMES_PER_BUFFER);
        int recBufSize = AudioRecord.getMinBufferSize(
                Integer.parseInt(nativeSampleRate),
                AudioFormat.CHANNEL_IN_MONO,
                AudioFormat.ENCODING_PCM_16BIT);
        supportRecording = true;
        if (recBufSize == AudioRecord.ERROR ||
            recBufSize == AudioRecord.ERROR_BAD_VALUE) {
            supportRecording = false;
        }
    }

    /**
     * Called whenever a key, touch, or trackball event is dispatched to the
     * activity.  Implement this method if you wish to know that the user has
     * interacted with the device in some way while your activity is running.
     * This callback and {@link #onUserLeaveHint} are intended to help
     * activities manage status bar notifications intelligently; specifically,
     * for helping activities determine the proper time to cancel a notfication.
     * <p/>
     * <p>All calls to your activity's {@link #onUserLeaveHint} callback will
     * be accompanied by calls to {@link #onUserInteraction}.  This
     * ensures that your activity will be told of relevant user activity such
     * as pulling down the notification pane and touching an item there.
     * <p/>
     * <p>Note that this callback will be invoked for the touch down action
     * that begins a touch gesture, but may not be invoked for the touch-moved
     * and touch-up actions that follow.
     *
     * @see #onUserLeaveHint()
     */
    @Override
    public void onUserInteraction() {
        updateNativeAudioUI();
    }

    private void updateNativeAudioUI() {
        if (!supportRecording) {
            status_view.setText("Error: Audio recording is not supported");
            return;
        }

        String msg = getNativeStatusMessage();
        if (msg != null && !msg.isEmpty()) {
            status_view.setText(msg);
        }
        else {
            status_view.setText("nativeSampleRate    = " + nativeSampleRate + "\n" +
                    "nativeSampleBufSize = " + nativeSampleBufSize);
        }
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

    public static native void startEchoProcessing();
    public static native void stopEchoProcessing();

    public static native String getNativeStatusMessage();
}
