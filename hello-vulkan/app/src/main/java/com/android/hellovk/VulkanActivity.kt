package com.android.hellovk

import android.app.NativeActivity
import android.os.Bundle

class VulkanActivity : NativeActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        System.loadLibrary("hellovkjni")
    }

    override fun onBackPressed() {
        System.gc()
        System.exit(0)
    }
}