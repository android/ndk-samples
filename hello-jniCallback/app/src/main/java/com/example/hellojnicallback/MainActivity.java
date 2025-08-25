/*
 * Copyright (C) 2016 The Android Open Source Project
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
package com.example.hellojnicallback;

import androidx.annotation.Keep;
import androidx.appcompat.app.AppCompatActivity;
import androidx.lifecycle.ViewModelProvider;
import androidx.lifecycle.ViewModelProvider.AndroidViewModelFactory;

import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity {

    MainViewModel model;
    TextView tickView;
    TextView helloJniMsg;
    Button pauseBtn;
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        tickView = findViewById(R.id.tickView);
        helloJniMsg = findViewById(R.id.hellojniMsg);
        pauseBtn = findViewById(R.id.pauseBtn);

        // Fetch the ViewModel, or have one instantiated
        model = new ViewModelProvider(this,
                AndroidViewModelFactory.getInstance(this.getApplication())
        ).get(MainViewModel.class);

        // if the timer has yet to be started, toggle it's state to trigger it's first run
        // otherwise, and only if it was previously running, start the jni thread for tick callbacks
        if(!model.started) {
            model.started = true;
            toggleTimerState();
        } else {
            if(model.running) {
                startTicks();
            }
        }

        setText();
    }

    /*
     * onDestroy gets called for configuration changes.
     * We make sure that the jni context cleans up so we don't lose track of threads.
     */
    @Override
    protected void onDestroy(){
        super.onDestroy();
        if(model.running) {
            stopTicks();
        }
    }

    private void toggleTimerState() {
        model.running = !model.running;
        setText();
        if(model.running){
            model.resetTimer();
            startTicks();
        } else {
            stopTicks();
        }
    }

    private void setText() {
        helloJniMsg.setText(stringFromJNI());
        tickView.setText(model.time());

        if(model.running) {
            pauseBtn.setText(R.string.pause);
        } else {
            pauseBtn.setText(R.string.resume);
        }


    }

    public void onPauseBtn(View v){
        toggleTimerState();
    }

    /*
     * A function calling from JNI to update current timer
     */
    @Keep
    private void updateTimer() {
        model.updateTimer();
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                MainActivity.this.tickView.setText(model.time());
            }
        });
    }
    static {
        System.loadLibrary("hello-jnicallback");
    }
    public native  String stringFromJNI();
    public native void startTicks();
    public native void stopTicks();
}
