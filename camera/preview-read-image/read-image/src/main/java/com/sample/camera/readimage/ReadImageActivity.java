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
 */
package com.sample.camera.readimage;

import android.Manifest;
import android.annotation.TargetApi;
import android.app.Activity;
import android.app.NativeActivity;
import android.content.pm.PackageManager;
import android.support.annotation.NonNull;
import android.support.v4.app.ActivityCompat;
import android.os.Bundle;
import android.util.Log;
import android.view.Surface;
import android.view.WindowManager;

import junit.framework.Assert;

public class ReadImageActivity extends NativeActivity
        implements ActivityCompat.OnRequestPermissionsResultCallback {
    ReadImageActivity savedInstance_;
    private final String DBG_TAG = "CAMERA-SAMPLE";
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.i(DBG_TAG, "OnCreate()");
        // new initialization here... request for permission
        savedInstance_  = this;
    }

    // get current rotation method
    int getRotationDegree() {
        return 90 * ((WindowManager)(getSystemService(WINDOW_SERVICE)))
                .getDefaultDisplay()
                .getRotation();
    }
    @Override
    protected void onResume() {
        super.onResume();
    }

    @Override
    protected void onPause() {
        super.onPause();
    }

    private static final int PERMISSION_REQUEST_CODE_CAMERA = 1;
    public void RequestCamera() {
        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.CAMERA) !=
                PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(
                    this,
                    new String[] { Manifest.permission.CAMERA },
                    PERMISSION_REQUEST_CODE_CAMERA);
            return;
        }
        notifyCameraPermission(true);
    }

    @Override
    public void onRequestPermissionsResult(int requestCode,
                                           @NonNull String[] permissions,
                                           @NonNull int[] grantResults) {
        /*
         * if any permission failed, the sample could not play
         */
        if (PERMISSION_REQUEST_CODE_CAMERA != requestCode) {
            super.onRequestPermissionsResult(requestCode,
                                             permissions,
                                             grantResults);
            return;
        }

        Assert.assertEquals(grantResults.length, 1);
        Assert.assertEquals(grantResults[0], PackageManager.PERMISSION_GRANTED);
        notifyCameraPermission(grantResults[0] == PackageManager.PERMISSION_GRANTED);
    }

    native static void notifyCameraPermission(boolean granted);

    static {
        System.loadLibrary("camera_view");
    }
}

