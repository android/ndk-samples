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

                externalNativeBuild {
                    cmake {
                        version = Versions.CMAKE
                    }
                }

                defaultConfig {
                    minSdk = Versions.MIN_SDK
                    lint {
                        targetSdk = Versions.TARGET_SDK
                    }
                    testOptions {
                        targetSdk = Versions.TARGET_SDK
                    }
                    externalNativeBuild {
                        cmake {
                            arguments.add("-DANDROID_SUPPORT_FLEXIBLE_PAGE_SIZES=ON")
                        }
                    }
                }
                compileOptions {
                    sourceCompatibility = Versions.JAVA
                    targetCompatibility = Versions.JAVA
                }

                // Studio will not automatically pass logcat through ndk-stack, so we need to avoid
                // stripping debug binaries if we want the crash trace to be readable.
                buildTypes {
                    debug {
                        packaging {
                            jniLibs {
                                keepDebugSymbols += "**/*.so"
                            }
                        }
                    }
                }
            }
        }
    }
}
