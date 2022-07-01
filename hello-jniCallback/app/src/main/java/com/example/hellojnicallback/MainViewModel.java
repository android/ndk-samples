package com.example.hellojnicallback;

import androidx.lifecycle.ViewModel;

public class MainViewModel extends ViewModel {
    private int hour = 0;
    private int minute = 0;
    private int second = 0;
    public boolean started = false;
    public boolean running = false;

    public void updateTimer() {
        ++second;
        if(second >= 60) {
            ++minute;
            second -= 60;
            if(minute >= 60) {
                ++hour;
                minute -= 60;
            }
        }
    }

    public void resetTimer() {
        hour = minute = second = 0;
    }

    public String time() {
        return hour + ":" + minute + ":" + second;
    }
}
