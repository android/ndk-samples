/*
 * Copyright 2013 The Android Open Source Project
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

package com.example.widecolor;

import android.annotation.SuppressLint;
import android.annotation.TargetApi;
import android.app.NativeActivity;
import android.content.Context;
import android.os.Bundle;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.Surface;
import android.view.View;
import android.view.ViewGroup.MarginLayoutParams;
import android.view.WindowManager;
import android.view.WindowManager.LayoutParams;
import android.widget.LinearLayout;
import android.widget.PopupWindow;
import android.widget.TextView;

public class WideColorActivity extends NativeActivity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        //Hide toolbar
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

    @TargetApi(19)
    protected void onResume() {
        super.onResume();
        setImmersiveSticky();
    }

    protected void onPause()
    {
        super.onPause();
    }

    // Our popup window, you will call it from your C/C++ code later

    @TargetApi(19)
    void setImmersiveSticky() {
        View decorView = getWindow().getDecorView();
        decorView.setSystemUiVisibility(View.SYSTEM_UI_FLAG_FULLSCREEN
                | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
                | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_LAYOUT_STABLE);
    }

    WideColorActivity _activity;
    PopupWindow _popupWindow;
    TextView _p3Image, _sRGBImage, _fileName;

    @SuppressLint("InflateParams")
    public void EnableUI(final int mask)
    {
        if( _popupWindow != null )
            return;

        _activity = this;

        this.runOnUiThread(new Runnable()  {
            @Override
            public void run()  {
                LayoutInflater layoutInflater
                = (LayoutInflater)getBaseContext()
                .getSystemService(LAYOUT_INFLATER_SERVICE);
                View popupView = layoutInflater.inflate(R.layout.widgets, null);
                _popupWindow = new PopupWindow(
                        popupView,
                        LayoutParams.MATCH_PARENT,
                        LayoutParams.WRAP_CONTENT);

                LinearLayout mainLayout = new LinearLayout(_activity);
                MarginLayoutParams params = new MarginLayoutParams(LayoutParams.MATCH_PARENT,
                        LayoutParams.WRAP_CONTENT);
                params.setMargins(0, 0, 0, 0);
                _activity.setContentView(mainLayout, params);

                // Show our UI over NativeActivity window
                _popupWindow.showAtLocation(mainLayout, Gravity.BOTTOM | Gravity.START, 0, 0);
                _popupWindow.update();

                _p3Image = (TextView)popupView.findViewById(R.id.textViewP3);
                _sRGBImage = (TextView)popupView.findViewById(R.id.textViewSRGB);
                _fileName = (TextView)popupView.findViewById(R.id.textViewWelcome);

                // Enable welcome message
                _p3Image.setVisibility(View.INVISIBLE);
                _sRGBImage.setVisibility(View.INVISIBLE);
                _fileName.setVisibility(View.VISIBLE);
            }});
    }

    public void EnableRenderUI(final String file) {
        this.runOnUiThread(new Runnable()  {
            @Override
            public void run() {
                _fileName.setText(file);
                _p3Image.setVisibility(View.VISIBLE);
                _sRGBImage.setVisibility(View.VISIBLE);
            }});
    }

    public void UpdateUI(final int mask, final String file)
    {
        _activity = this;
        this.runOnUiThread(new Runnable()  {
            @Override
            public void run()  {
                if(_p3Image == null || _sRGBImage == null || _fileName == null)
                    return;

                _p3Image.setVisibility(((mask & 0x01) == 0x01) ? View.VISIBLE : View.INVISIBLE);
                _sRGBImage.setVisibility((mask & 0x02) == 0x02 ? View.VISIBLE : View.INVISIBLE);
                _fileName.setText(file);
            }});
    }

    public int GetRotation() {
        int rotation = ((WindowManager) getSystemService(Context.WINDOW_SERVICE))
                .getDefaultDisplay()
                .getRotation();
        int angle = 0;
        switch (rotation) {
            case (Surface.ROTATION_90) :
                angle = 90;
                break;
            case (Surface.ROTATION_180) :
                angle = 180;
                break;
            case (Surface.ROTATION_270):
                angle = 270;
                break;
            case (Surface.ROTATION_0) :
            default:
                angle = 0;
                break;
        }
        return angle;
    }
}


