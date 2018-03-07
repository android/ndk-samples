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
import android.os.Handler;
import android.view.Display;
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
    final int ONE_SECOND  = 1000;
    final int DISPLAY_FOREVER = -1;

    final int LEGEND_P3_IMAGE_BIT = 0x01;
    final int LEGEND_SRGB_IMAGE_BIT = 0x02;
    final int LEGEND_FILENAME_BIT = 0x04;
    final int LEGEND_DISPLAY_ALL = LEGEND_P3_IMAGE_BIT | LEGEND_SRGB_IMAGE_BIT |
                                   LEGEND_FILENAME_BIT;

    View _popupView;
    TextView _p3Image, _sRGBImage, _fileName;
    LinearLayout _mainLayout;
    volatile PopupWindow _popupWindow;
    volatile boolean _dismissPending;
    Handler _handler;

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

        LayoutInflater layoutInflater
                = (LayoutInflater)getBaseContext()
                .getSystemService(LAYOUT_INFLATER_SERVICE);
        _popupView = layoutInflater.inflate(R.layout.widgets, null);
        _p3Image = (TextView)_popupView.findViewById(R.id.textViewP3);
        _sRGBImage = (TextView)_popupView.findViewById(R.id.textViewSRGB);
        _fileName = (TextView)_popupView.findViewById(R.id.textViewWelcome);

        _mainLayout = new LinearLayout(this);
        MarginLayoutParams params = new MarginLayoutParams(LayoutParams.MATCH_PARENT,
                LayoutParams.WRAP_CONTENT);
        params.setMargins(0, 0, 0, 0);
        setContentView(_mainLayout, params);
        _popupWindow = null;

        _handler = new Handler();
        _dismissPending = false;
    }

    @TargetApi(26)
    protected void onResume() {
        super.onResume();
        setImmersiveSticky();
    }

    protected void onPause()
    {
        super.onPause();
    }

    // Our popup window, you will call it from your C/C++ code later
    @TargetApi(26)
    void setImmersiveSticky() {
        View decorView = getWindow().getDecorView();
        decorView.setSystemUiVisibility(View.SYSTEM_UI_FLAG_FULLSCREEN
                | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
                | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_LAYOUT_STABLE);
    }

    /**
     * Enable/Disable popupWindow. The popupWindow would display for
     * ONE_SECOND, then dismissed; if swiping is faster than ONE_SECOND
     * the latest one is displayed, but with less time.
     * @param mask is a bitmask of 3 TextViews ( P3, sRGB, and fileName)
     * @param file is the file name to be displayed
     * @param duration is time to display 3 TextViews in milisecond.
     */
    void DisplayLegend(final int mask, final String file, int duration) {
        if (_dismissPending) {
            if (file != null) {
                _fileName.setText(file);
            }
            return;
        }
        if (_popupWindow == null) {
            _popupWindow = new PopupWindow(
                    _popupView,
                    LayoutParams.MATCH_PARENT,
                    LayoutParams.WRAP_CONTENT);
            // Show our UI over NativeActivity window
            _popupWindow.showAtLocation(_mainLayout, Gravity.BOTTOM | Gravity.START, 0, 0);
            _popupWindow.update();
        }

        _p3Image.setVisibility(((mask & LEGEND_P3_IMAGE_BIT) != 0) ? View.VISIBLE : View.INVISIBLE);
        _sRGBImage.setVisibility(((mask & LEGEND_SRGB_IMAGE_BIT) != 0) ? View.VISIBLE : View.INVISIBLE);
        if (file != null) {
            _fileName.setText(file);
        }
        _fileName.setVisibility(((mask & LEGEND_FILENAME_BIT) != 0)? View.VISIBLE: View.INVISIBLE);

        if (duration != DISPLAY_FOREVER) {
            _handler.postDelayed(new Runnable() {
                final PopupWindow popupWindow = _popupWindow;
                @Override
                public void run() {
                    popupWindow.dismiss();
                    _dismissPending = false;
                }
            }, duration);
            _popupWindow = null;
            _dismissPending = true;
        }
    }
    @SuppressLint("InflateParams")
    public void EnableUI(final int mask)
    {
        if( _popupWindow != null )
            return;

        this.runOnUiThread(new Runnable()  {
            @Override
            public void run()  {
                // Show our UI over NativeActivity window
                DisplayLegend(LEGEND_FILENAME_BIT, null, DISPLAY_FOREVER);
            }});
    }

    public void EnableRenderUI(final String file) {
        this.runOnUiThread(new Runnable()  {
            @Override
            public void run() {
                DisplayLegend(LEGEND_DISPLAY_ALL, file, DISPLAY_FOREVER);
            }});
    }

    public void UpdateUI(final int mask, final String file)
    {
        boolean displaying = (mask & 0x07) != 0;
        if ((mask & LEGEND_DISPLAY_ALL) == 0) {
            return;
        }

        this.runOnUiThread(new Runnable()  {
            @Override
            public void run()  {
            int newMask = mask;
            if (file != null) {
                newMask |= LEGEND_FILENAME_BIT;
            }
            DisplayLegend(newMask, file, ONE_SECOND);
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


