package com.example.native_softkeyboard;

import android.app.NativeActivity;

public class ActivityHelper {
    static public void setContentView(final NativeActivity activity, final int layoutID) {
        activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                activity.setContentView(layoutID);
            }
        });
    }
}