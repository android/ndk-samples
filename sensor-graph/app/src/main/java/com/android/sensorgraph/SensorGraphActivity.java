/*
 * Copyright (C) 2007 The Android Open Source Project
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

package com.android.sensorgraph;

import android.app.Activity;
import android.opengl.GLSurfaceView;
import android.os.Bundle;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;


public class SensorGraphActivity extends Activity {

    GLSurfaceView mView;

    @Override protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        mView = new GLSurfaceView(getApplication());
        mView.setEGLContextClientVersion(2);
        mView.setRenderer(new GLSurfaceView.Renderer() {
            @Override
            public void onSurfaceCreated(GL10 gl, EGLConfig config) {
                SensorGraphJNI.surfaceCreated();
            }

            @Override
            public void onSurfaceChanged(GL10 gl, int width, int height) {
                SensorGraphJNI.surfaceChanged(width, height);
            }

            @Override
            public void onDrawFrame(GL10 gl) {
                SensorGraphJNI.drawFrame();
            }
        });
        mView.queueEvent(new Runnable() {
            @Override
            public void run() {
                SensorGraphJNI.init(getAssets());
            }
        });
	    setContentView(mView);
    }

    @Override protected void onPause() {
        super.onPause();
        mView.onPause();
        mView.queueEvent(new Runnable() {
            @Override
            public void run() {
                SensorGraphJNI.pause();
            }
        });
    }

    @Override protected void onResume() {
        super.onResume();
        mView.onResume();
        mView.queueEvent(new Runnable() {
            @Override
            public void run() {
                SensorGraphJNI.resume();
            }
        });
    }
}
