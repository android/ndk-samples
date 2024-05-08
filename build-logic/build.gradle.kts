import org.jetbrains.kotlin.gradle.tasks.KotlinCompile

plugins {
    id("java-gradle-plugin")
    `kotlin-dsl`
    alias(libs.plugins.jetbrains.kotlin.jvm)
}

java {
    sourceCompatibility = JavaVersion.VERSION_17
    targetCompatibility = JavaVersion.VERSION_17
}

tasks.withType<KotlinCompile>().configureEach {
    kotlinOptions {
        jvmTarget = JavaVersion.VERSION_17.toString()
    }
}

dependencies {
    compileOnly(libs.android.gradlePlugin)
    compileOnly(libs.kotlin.gradlePlugin)
}

gradlePlugin {
    plugins {
        register("androidApplication") {
            id = "ndksamples.android.application"
            implementationClass = "com.android.ndk.samples.buildlogic.AndroidApplicationConventionPlugin"
        }
        register("androidLibrary") {
            id = "ndksamples.android.library"
            implementationClass = "com.android.ndk.samples.buildlogic.AndroidLibraryConventionPlugin"
        }
        register("kotlinAndroid") {
            id = "ndksamples.android.kotlin"
            implementationClass = "com.android.ndk.samples.buildlogic.KotlinConventionPlugin"
        }
    }
}
