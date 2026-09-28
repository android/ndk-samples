package com.android.ndk.samples.buildlogic

import com.android.build.api.dsl.LibraryExtension
import org.gradle.api.JavaVersion
import org.gradle.api.Plugin
import org.gradle.api.Project
import org.gradle.api.artifacts.VersionCatalogsExtension
import org.gradle.kotlin.dsl.configure
import org.gradle.kotlin.dsl.getByType

class AndroidLibraryConventionPlugin : Plugin<Project> {
    override fun apply(target: Project) {
        val libs = target.extensions.getByType<VersionCatalogsExtension>().named("libs")
        val compileSdkVersion = libs.findVersion("compileSdk").get().requiredVersion.toInt()
        val minSdkVersion = libs.findVersion("minSdk").get().requiredVersion.toInt()
        val targetSdkVersion = libs.findVersion("targetSdk").get().requiredVersion.toInt()
        val ndkVersionStr = libs.findVersion("ndk").get().requiredVersion
        val cmakeVersionStr = libs.findVersion("cmake").get().requiredVersion
        val javaVersion = JavaVersion.toVersion(libs.findVersion("javaTarget").get().requiredVersion)

        with(target) {
            with(pluginManager) {
                apply("com.android.library")
            }

            extensions.configure<LibraryExtension> {
                compileSdk = compileSdkVersion
                ndkVersion = ndkVersionStr

                externalNativeBuild {
                    cmake {
                        version = cmakeVersionStr
                    }
                }

                defaultConfig {
                    minSdk = minSdkVersion
                    lint {
                        targetSdk = targetSdkVersion
                    }
                    testOptions {
                        targetSdk = targetSdkVersion
                    }
                    externalNativeBuild {
                        cmake {
                            arguments.add("-DANDROID_SUPPORT_FLEXIBLE_PAGE_SIZES=ON")
                            arguments.add("-DCMAKE_MODULE_PATH=${rootDir.resolve("cmake")}")
                        }
                    }
                    ndk {
                        // riscv64 isn't a supported Android ABI yet (August 2025), but we're
                        // enabling it here as part of that experiment. Until it's a supported ABI,
                        // don't include this in your app, as Play will block uploads of APKs which
                        // contain riscv64 libraries.
                        abiFilters.addAll(
                            listOf(
                                "arm64-v8a",
                                "armeabi-v7a",
                                "riscv64",
                                "x86",
                                "x86_64",
                            )
                        )
                    }
                }
                compileOptions {
                    sourceCompatibility = javaVersion
                    targetCompatibility = javaVersion
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
