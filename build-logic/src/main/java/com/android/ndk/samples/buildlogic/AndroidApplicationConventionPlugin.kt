package com.android.ndk.samples.buildlogic

import com.android.build.api.dsl.ApplicationExtension

import org.gradle.api.Plugin
import org.gradle.api.Project
import org.gradle.kotlin.dsl.configure

class AndroidApplicationConventionPlugin : Plugin<Project> {
    override fun apply(target: Project) {
        with(target) {
            with(pluginManager) {
                apply("com.android.application")
            }

            extensions.configure<ApplicationExtension> {
                compileSdk = Versions.COMPILE_SDK
                ndkVersion = Versions.NDK
                defaultConfig {
                    minSdk = Versions.MIN_SDK
                    targetSdk = Versions.TARGET_SDK
                }
                compileOptions {
                    sourceCompatibility = Versions.JAVA
                    targetCompatibility = Versions.JAVA
                }
            }
        }
    }
}
