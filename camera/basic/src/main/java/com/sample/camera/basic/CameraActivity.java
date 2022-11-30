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
import android.content.Context;
import android.content.pm.PackageManager;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraManager;
import androidx.annotation.NonNull;
import androidx.core.app.ActivityCompat;
import android.os.Bundle;
import android.util.Log;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.ImageButton;
import android.widget.PopupWindow;
import android.widget.RelativeLayout;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;

import static android.hardware.camera2.CameraMetadata.LENS_FACING_BACK;

class CameraSeekBar {
    int _progress;
    long _min, _max, _absVal;
    SeekBar _seekBar;
    TextView _sliderPrompt;
    CameraSeekBar() {
        _progress = 0;
        _min = _max = _absVal  = 0;
    }

    CameraSeekBar(SeekBar seekBar, TextView textView, long min, long max, long val) {
        _seekBar = seekBar;
        _sliderPrompt = textView;
        _min = min;
        _max = max;
        _absVal = val;

        if(_min != _max) {
            _progress = (int) ((_absVal - _min) * _seekBar.getMax() / (_max - _min));
            seekBar.setProgress(_progress);
            updateProgress(_progress);
        } else {
            _progress = 0;
            seekBar.setEnabled(false);
        }
    }

    public boolean isSupported() {
        return (_min != _max);
    }
    public void updateProgress(int progress) {
        if (!isSupported())
            return;

        _progress = progress;
        _absVal = (progress * ( _max - _min )) / _seekBar.getMax() + _min;
        int val = (progress * (_seekBar.getWidth() - 2 * _seekBar.getThumbOffset())) / _seekBar.getMax();
        _sliderPrompt.setText("" + _absVal);
        _sliderPrompt.setX(_seekBar.getX() + val + _seekBar.getThumbOffset() / 2);
    }
    public int getProgress() {
        return _progress;
    }
    public void updateAbsProgress(long val) {
        if (!isSupported())
            return;
        int progress = (int)((val - _min) * _seekBar.getMax() / (_max - _min));
        updateProgress(progress);
    }
    public long getAbsProgress() {
        return _absVal;
    }
}

public class CameraActivity extends NativeActivity
        implements ActivityCompat.OnRequestPermissionsResultCallback {
    volatile CameraActivity _savedInstance;
    PopupWindow _popupWindow;
    ImageButton _takePhoto;
    CameraSeekBar _exposure, _sensitivity;
    long[] _initParams;

    private final String DBG_TAG = "NDK-CAMERA-BASIC";
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

    private boolean isCamera2Device() {
        CameraManager camMgr = (CameraManager)getSystemService(Context.CAMERA_SERVICE);
        boolean camera2Dev = true;
        try {
            String[] cameraIds = camMgr.getCameraIdList();
            if (cameraIds.length != 0 ) {
                for (String id : cameraIds) {
                    CameraCharacteristics characteristics = camMgr.getCameraCharacteristics(id);
                    int deviceLevel = characteristics.get(CameraCharacteristics.INFO_SUPPORTED_HARDWARE_LEVEL);
                    int facing = characteristics.get(CameraCharacteristics.LENS_FACING);
                    if (deviceLevel == CameraCharacteristics.INFO_SUPPORTED_HARDWARE_LEVEL_LEGACY &&
                        facing == LENS_FACING_BACK) {
                        camera2Dev =  false;
                    }
                }
            }
        } catch (CameraAccessException e) {
            e.printStackTrace();
            camera2Dev = false;
        }
        return camera2Dev;
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
        if (_popupWindow != null && _popupWindow.isShowing()) {
            _popupWindow.dismiss();
            _popupWindow = null;
        }
        super.onPause();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
    }

    private static final int PERMISSION_REQUEST_CODE_CAMERA = 1;
    public void RequestCamera() {
        if(!isCamera2Device()) {
            Log.e(DBG_TAG, "Found legacy camera Device, this sample needs camera2 device");
            return;
        }
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

        if(grantResults.length == 2) {
            notifyCameraPermission(grantResults[0] == PackageManager.PERMISSION_GRANTED &&
                    grantResults[1] == PackageManager.PERMISSION_GRANTED);
        }
    }

    /**
     * params[] exposure and sensitivity init values in (min, max, curVa) tuple
     *   0: exposure min
     *   1: exposure max
     *   2: exposure val
     *   3: sensitivity min
     *   4: sensitivity max
     *   5: sensitivity val
     */
    @SuppressLint("InflateParams")
    public void EnableUI(final long[] params)
    {
        // make our own copy
        _initParams = new long[params.length];
        System.arraycopy(params, 0, _initParams, 0, params.length);

        runOnUiThread(new Runnable()  {
            @Override
            public void run()  {
                try {
                    if (_popupWindow != null) {
                        _popupWindow.dismiss();
                    }
                    LayoutInflater layoutInflater
                            = (LayoutInflater) getBaseContext()
                            .getSystemService(LAYOUT_INFLATER_SERVICE);
                    View popupView = layoutInflater.inflate(R.layout.widgets, null);
                    _popupWindow = new PopupWindow(
                            popupView,
                            WindowManager.LayoutParams.MATCH_PARENT,
                            WindowManager.LayoutParams.WRAP_CONTENT);

                    RelativeLayout mainLayout = new RelativeLayout(_savedInstance);
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
                    (popupView.findViewById(R.id.exposureLabel)).setEnabled(true);
                    (popupView.findViewById(R.id.sensitivityLabel)).setEnabled(true);

                    SeekBar seekBar = (SeekBar) popupView.findViewById(R.id.exposure_seekbar);
                    _exposure = new CameraSeekBar(seekBar,
                            (TextView) popupView.findViewById(R.id.exposureVal),
                            _initParams[0], _initParams[1], _initParams[2]);
                    seekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
                        @Override
                        public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                            _exposure.updateProgress(progress);
                            OnExposureChanged(_exposure.getAbsProgress());
                        }

                        @Override
                        public void onStartTrackingTouch(SeekBar seekBar) {
                        }

                        @Override
                        public void onStopTrackingTouch(SeekBar seekBar) {
                        }
                    });
                    seekBar = ((SeekBar) popupView.findViewById(R.id.sensitivity_seekbar));
                    _sensitivity = new CameraSeekBar(seekBar,
                            (TextView) popupView.findViewById(R.id.sensitivityVal),
                            _initParams[3], _initParams[4], _initParams[5]);
                    seekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
                        @Override
                        public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                            _sensitivity.updateProgress(progress);
                            OnSensitivityChanged(_sensitivity.getAbsProgress());
                        }

                        @Override
                        public void onStartTrackingTouch(SeekBar seekBar) {
                        }

                        @Override
                        public void onStopTrackingTouch(SeekBar seekBar) {
                        }
                    });
                } catch (WindowManager.BadTokenException e) {
                    // UI error out, ignore and continue
                    Log.e(DBG_TAG, "UI Exception Happened: " + e.getMessage());
                }
            }});
    }
    /**
      Called from Native side to notify that a photo is taken
     */
    public void OnPhotoTaken(String fileName) {
        final String name = fileName;
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                Toast.makeText(getApplicationContext(),
                        "Photo saved to " + name, Toast.LENGTH_SHORT).show();
            }
        });
    }

    native static void notifyCameraPermission(boolean granted);
    native static void TakePhoto();
    native void OnExposureChanged(long exposure);
    native void OnSensitivityChanged(long sensitivity);

    static {
        System.loadLibrary("ndk_camera");
    }
}

