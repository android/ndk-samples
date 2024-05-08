package com.android.ndk.samples.buildlogic

import com.android.build.api.dsl.LibraryExtension
import org.gradle.api.Plugin
import org.gradle.api.Project
import org.gradle.kotlin.dsl.configure

class AndroidLibraryConventionPlugin : Plugin<Project> {
    override fun apply(target: Project) {
        with(target) {
            with(pluginManager) {
                apply("com.android.library")
            }

            extensions.configure<LibraryExtension> {
                compileSdk = Versions.COMPILE_SDK
                ndkVersion = Versions.NDK
                defaultConfig {
                    minSdk = Versions.MIN_SDK
                    lint {
                        targetSdk = Versions.TARGET_SDK
                    }
                    testOptions {
                        targetSdk = Versions.TARGET_SDK
                    }
                }
                compileOptions {
                    sourceCompatibility = Versions.JAVA
                    targetCompatibility = Versions.JAVA
                }
            }
        }
    }
}
