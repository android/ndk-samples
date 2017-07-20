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
package com.sample.camera.basic;

import android.Manifest;
import android.annotation.SuppressLint;
import android.app.NativeActivity;
import android.content.pm.PackageManager;
import android.support.annotation.NonNull;
import android.support.v4.app.ActivityCompat;
import android.os.Bundle;
import android.util.Log;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.ImageButton;
import android.widget.LinearLayout;
import android.widget.PopupWindow;

import junit.framework.Assert;

public class CameraActivity extends NativeActivity
        implements ActivityCompat.OnRequestPermissionsResultCallback {
    CameraActivity _savedInstance;
    PopupWindow _popupWindow;
    ImageButton _takePhoto;
    private final String DBG_TAG = "CAMERA-SAMPLE";
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.i(DBG_TAG, "OnCreate()");
        // new initialization here... request for permission
        _savedInstance  = this;

        setImmersiveSticky();
        View decorView = getWindow().getDecorView();
        decorView.setOnSystemUiVisibilityChangeListener
                (new View.OnSystemUiVisibilityChangeListener() {
                    @Override
                    public void onSystemUiVisibilityChange(int visibility) {
                        setImmersiveSticky();
                    }
                });
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
        setImmersiveSticky();
    }
    void setImmersiveSticky() {
        View decorView = getWindow().getDecorView();
        decorView.setSystemUiVisibility(View.SYSTEM_UI_FLAG_FULLSCREEN
                | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
                | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_LAYOUT_STABLE);
    }

    @Override
    protected void onPause() {
        super.onPause();
    }

    private static final int PERMISSION_REQUEST_CODE_CAMERA = 1;
    public void RequestCamera() {
        String[] accessPermissions = new String[] {
            Manifest.permission.CAMERA,
            Manifest.permission.WRITE_EXTERNAL_STORAGE
        };
        boolean needRequire  = false;
        for(String access : accessPermissions) {
           int curPermission = ActivityCompat.checkSelfPermission(this, access);
           if(curPermission != PackageManager.PERMISSION_GRANTED) {
               needRequire = true;
               break;
           }
        }
        if (needRequire) {
            ActivityCompat.requestPermissions(
                    this,
                    accessPermissions,
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

        Assert.assertEquals(grantResults.length, 2);
        notifyCameraPermission(grantResults[0] == PackageManager.PERMISSION_GRANTED &&
                               grantResults[1] == PackageManager.PERMISSION_GRANTED);
    }

    @SuppressLint("InflateParams")
    public void EnableUI()
    {
        if( _popupWindow != null )
            return;

        runOnUiThread(new Runnable()  {
            @Override
            public void run()  {
                LayoutInflater layoutInflater
                        = (LayoutInflater)getBaseContext()
                        .getSystemService(LAYOUT_INFLATER_SERVICE);
                View popupView = layoutInflater.inflate(R.layout.widgets, null);
                _popupWindow = new PopupWindow(
                        popupView,
                        WindowManager.LayoutParams.MATCH_PARENT,
                        WindowManager.LayoutParams.WRAP_CONTENT);

                LinearLayout mainLayout = new LinearLayout(_savedInstance);
                ViewGroup.MarginLayoutParams params = new ViewGroup.MarginLayoutParams(
                        -1, -1);
                params.setMargins(0, 0, 0, 0);
                _savedInstance.setContentView(mainLayout, params);

                // Show our UI over NativeActivity window
                _popupWindow.showAtLocation(mainLayout, Gravity.BOTTOM | Gravity.START, 0, 0);
                _popupWindow.update();

                _takePhoto = (ImageButton) popupView.findViewById(R.id.takePhoto);
                _takePhoto.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        TakePhoto();
                    }
                });
                _takePhoto.setEnabled(true);
            }});
    }


    native static void notifyCameraPermission(boolean granted);
    native static void TakePhoto();

    static {
        System.loadLibrary("camera_view");
    }
}

