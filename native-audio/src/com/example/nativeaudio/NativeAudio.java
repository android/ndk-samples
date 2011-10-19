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

package com.example.nativeaudio;

import android.app.Activity;
import android.content.res.AssetManager;
import android.os.Bundle;
//import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.Spinner;
import android.widget.Toast;

public class NativeAudio extends Activity {

    //static final String TAG = "NativeAudio";

    static final int CLIP_NONE = 0;
    static final int CLIP_HELLO = 1;
    static final int CLIP_ANDROID = 2;
    static final int CLIP_SAWTOOTH = 3;
    static final int CLIP_PLAYBACK = 4;

    static String URI;
    static AssetManager assetManager;

    static boolean isPlayingAsset = false;
    static boolean isPlayingUri = false;

    static int numChannelsUri = 0;

    /** Called when the activity is first created. */
    @Override
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        setContentView(R.layout.main);

        assetManager = getAssets();

        // initialize native audio system

        createEngine();
        createBufferQueueAudioPlayer();

        // initialize URI spinner
        Spinner uriSpinner = (Spinner) findViewById(R.id.uri_spinner);
        ArrayAdapter<CharSequence> uriAdapter = ArrayAdapter.createFromResource(
                this, R.array.uri_spinner_array, android.R.layout.simple_spinner_item);
        uriAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        uriSpinner.setAdapter(uriAdapter);
        uriSpinner.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {

            public void onItemSelected(AdapterView<?> parent, View view, int pos, long id) {
                URI = parent.getItemAtPosition(pos).toString();
            }

            public void onNothingSelected(AdapterView parent) {
                URI = null;
            }

        });

        // initialize button click handlers

        ((Button) findViewById(R.id.hello)).setOnClickListener(new OnClickListener() {
            public void onClick(View view) {
                // ignore the return value
                selectClip(CLIP_HELLO, 5);
            }
        });

        ((Button) findViewById(R.id.android)).setOnClickListener(new OnClickListener() {
            public void onClick(View view) {
                // ignore the return value
                selectClip(CLIP_ANDROID, 7);
            }
        });

        ((Button) findViewById(R.id.sawtooth)).setOnClickListener(new OnClickListener() {
            public void onClick(View view) {
                // ignore the return value
                selectClip(CLIP_SAWTOOTH, 1);
            }
        });

        ((Button) findViewById(R.id.reverb)).setOnClickListener(new OnClickListener() {
            boolean enabled = false;
            public void onClick(View view) {
                enabled = !enabled;
                if (!enableReverb(enabled)) {
                    enabled = !enabled;
                }
            }
        });

        ((Button) findViewById(R.id.embedded_soundtrack)).setOnClickListener(new OnClickListener() {
            boolean created = false;
            public void onClick(View view) {
                if (!created) {
                    created = createAssetAudioPlayer(assetManager, "background.mp3");
                }
                if (created) {
                    isPlayingAsset = !isPlayingAsset;
                    setPlayingAssetAudioPlayer(isPlayingAsset);
                }
            }
        });

        ((Button) findViewById(R.id.uri_soundtrack)).setOnClickListener(new OnClickListener() {
            boolean created = false;
            public void onClick(View view) {
                if (!created && URI != null) {
                    created = createUriAudioPlayer(URI);
                }
             }
        });

        ((Button) findViewById(R.id.pause_uri)).setOnClickListener(new OnClickListener() {
            public void onClick(View view) {
                setPlayingUriAudioPlayer(false);
             }
        });

        ((Button) findViewById(R.id.play_uri)).setOnClickListener(new OnClickListener() {
            public void onClick(View view) {
                setPlayingUriAudioPlayer(true);
             }
        });

        ((Button) findViewById(R.id.loop_uri)).setOnClickListener(new OnClickListener() {
            boolean isLooping = false;
            public void onClick(View view) {
                isLooping = !isLooping;
                setLoopingUriAudioPlayer(isLooping);
             }
        });

        ((Button) findViewById(R.id.mute_left_uri)).setOnClickListener(new OnClickListener() {
            boolean muted = false;
            public void onClick(View view) {
                muted = !muted;
                setChannelMuteUriAudioPlayer(0, muted);
             }
        });

        ((Button) findViewById(R.id.mute_right_uri)).setOnClickListener(new OnClickListener() {
            boolean muted = false;
            public void onClick(View view) {
                muted = !muted;
                setChannelMuteUriAudioPlayer(1, muted);
             }
        });

        ((Button) findViewById(R.id.solo_left_uri)).setOnClickListener(new OnClickListener() {
            boolean soloed = false;
            public void onClick(View view) {
                soloed = !soloed;
                setChannelSoloUriAudioPlayer(0, soloed);
             }
        });

        ((Button) findViewById(R.id.solo_right_uri)).setOnClickListener(new OnClickListener() {
            boolean soloed = false;
            public void onClick(View view) {
                soloed = !soloed;
                setChannelSoloUriAudioPlayer(1, soloed);
             }
        });

        ((Button) findViewById(R.id.mute_uri)).setOnClickListener(new OnClickListener() {
            boolean muted = false;
            public void onClick(View view) {
                muted = !muted;
                setMuteUriAudioPlayer(muted);
             }
        });

        ((Button) findViewById(R.id.enable_stereo_position_uri)).setOnClickListener(
                new OnClickListener() {
            boolean enabled = false;
            public void onClick(View view) {
                enabled = !enabled;
                enableStereoPositionUriAudioPlayer(enabled);
             }
        });

        ((Button) findViewById(R.id.channels_uri)).setOnClickListener(new OnClickListener() {
            public void onClick(View view) {
                if (numChannelsUri == 0) {
                    numChannelsUri = getNumChannelsUriAudioPlayer();
                }
                Toast.makeText(NativeAudio.this, "Channels: " + numChannelsUri,
                        Toast.LENGTH_SHORT).show();
             }
        });

        ((SeekBar) findViewById(R.id.volume_uri)).setOnSeekBarChangeListener(
                new OnSeekBarChangeListener() {
            int lastProgress = 100;
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                assert progress >= 0 && progress <= 100;
                lastProgress = progress;
            }
            public void onStartTrackingTouch(SeekBar seekBar) {
            }
            public void onStopTrackingTouch(SeekBar seekBar) {
                int attenuation = 100 - lastProgress;
                int millibel = attenuation * -50;
                setVolumeUriAudioPlayer(millibel);
            }
        });

        ((SeekBar) findViewById(R.id.pan_uri)).setOnSeekBarChangeListener(
                new OnSeekBarChangeListener() {
            int lastProgress = 100;
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                assert progress >= 0 && progress <= 100;
                lastProgress = progress;
            }
            public void onStartTrackingTouch(SeekBar seekBar) {
            }
            public void onStopTrackingTouch(SeekBar seekBar) {
                int permille = (lastProgress - 50) * 20;
                setStereoPositionUriAudioPlayer(permille);
            }
        });

        ((Button) findViewById(R.id.record)).setOnClickListener(new OnClickListener() {
            boolean created = false;
            public void onClick(View view) {
                if (!created) {
                    created = createAudioRecorder();
                }
                if (created) {
                    startRecording();
                }
            }
        });

        ((Button) findViewById(R.id.playback)).setOnClickListener(new OnClickListener() {
            public void onClick(View view) {
                // ignore the return value
                selectClip(CLIP_PLAYBACK, 3);
            }
        });

    }

    /** Called when the activity is about to be destroyed. */
    @Override
    protected void onPause()
    {
        // turn off all audio
        selectClip(CLIP_NONE, 0);
        isPlayingAsset = false;
        setPlayingAssetAudioPlayer(false);
        isPlayingUri = false;
        setPlayingUriAudioPlayer(false);
        super.onPause();
    }

    /** Called when the activity is about to be destroyed. */
    @Override
    protected void onDestroy()
    {
        shutdown();
        super.onDestroy();
    }

    /** Native methods, implemented in jni folder */
    public static native void createEngine();
    public static native void createBufferQueueAudioPlayer();
    public static native boolean createAssetAudioPlayer(AssetManager assetManager, String filename);
    // true == PLAYING, false == PAUSED
    public static native void setPlayingAssetAudioPlayer(boolean isPlaying);
    public static native boolean createUriAudioPlayer(String uri);
    public static native void setPlayingUriAudioPlayer(boolean isPlaying);
    public static native void setLoopingUriAudioPlayer(boolean isLooping);
    public static native void setChannelMuteUriAudioPlayer(int chan, boolean mute);
    public static native void setChannelSoloUriAudioPlayer(int chan, boolean solo);
    public static native int getNumChannelsUriAudioPlayer();
    public static native void setVolumeUriAudioPlayer(int millibel);
    public static native void setMuteUriAudioPlayer(boolean mute);
    public static native void enableStereoPositionUriAudioPlayer(boolean enable);
    public static native void setStereoPositionUriAudioPlayer(int permille);
    public static native boolean selectClip(int which, int count);
    public static native boolean enableReverb(boolean enabled);
    public static native boolean createAudioRecorder();
    public static native void startRecording();
    public static native void shutdown();

    /** Load jni .so on initialization */
    static {
         System.loadLibrary("native-audio-jni");
    }

}
