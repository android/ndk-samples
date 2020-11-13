/*
 * Copyright (C) 2019 The Android Open Source Project
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
 *
 */

package com.google.example.hellooboe

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.view.MotionEvent
import android.view.View
import android.widget.Toast
import kotlinx.android.synthetic.main.activity_main.sample_text

class MainActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
    }

    /*
    * Hook to user control to start / stop audio playback:
    *    touch-down: start, and keeps on playing
    *    touch-up: stop.
    * simply pass the events to native side.
    */
    override fun onTouchEvent(event: MotionEvent): Boolean {
        when (event.actionMasked) {
            MotionEvent.ACTION_DOWN -> playSound(true)
            MotionEvent.ACTION_UP -> playSound(false)
        }
        return super.onTouchEvent(event)
    }

    override fun onResume() {
        super.onResume()
        if (createStream() != 0) {
            val errorString : String = getString(R.string.error_msg)
            Toast.makeText(applicationContext, errorString,Toast.LENGTH_LONG).show()
            sample_text.text = errorString
        }
    }

    override fun onPause() {
        destroyStream()
        super.onPause()
    }

    // Creates and starts Oboe stream to play audio
    private external fun createStream() : Int

    // Closes and destroys Oboe stream when app goes out of focus
    private external fun destroyStream()

    // Plays sound on user tap
    private external fun playSound(enable: Boolean) : Int

    companion object {
        // Used to load native code calling oboe on app startup.
        init {
            System.loadLibrary("hello-oboe")
        }
    }
}
