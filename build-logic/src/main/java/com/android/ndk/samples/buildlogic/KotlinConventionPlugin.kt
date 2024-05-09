package com.android.ndk.samples.buildlogic

import com.android.build.api.dsl.ApplicationExtension

import org.gradle.api.Plugin
import org.gradle.api.Project
import org.gradle.kotlin.dsl.configure
import org.gradle.kotlin.dsl.withType
import org.jetbrains.kotlin.gradle.tasks.KotlinCompile

class KotlinConventionPlugin : Plugin<Project> {
    override fun apply(target: Project) {
        with(target) {
            with(pluginManager) {
                apply("org.jetbrains.kotlin.android")
            }

            extensions.configure<ApplicationExtension> {
                tasks.withType<KotlinCompile>().configureEach {
                    kotlinOptions {
                        jvmTarget = Versions.JAVA.toString()
                    }
                }
            }
        }
    }
}
