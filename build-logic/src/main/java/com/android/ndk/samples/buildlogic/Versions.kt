package com.android.ndk.samples.buildlogic

import org.gradle.api.JavaVersion

object Versions {
    const val COMPILE_SDK = 35
    const val TARGET_SDK = 35
    const val MIN_SDK = 21
    const val NDK = "28.2.13676358" // r28c
    const val CMAKE = "4.1.0"
    val JAVA = JavaVersion.VERSION_1_8
}
