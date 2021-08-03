package com.raptrmatt.hellogl2kotlin

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle

class MainActivity : AppCompatActivity() {
    private lateinit var mView: GL2JNIView

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        mView = GL2JNIView(application)
        setContentView(mView)
    }

    override fun onPause() {
        super.onPause()
        mView.onPause()
    }

    override fun onResume() {
        super.onResume()
        mView.onResume()
    }
}