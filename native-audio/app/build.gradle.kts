plugins {
    id("ndksamples.android.application")
}

android {
    namespace = "com.example.nativeaudio"

    defaultConfig {
        applicationId = "com.example.nativeaudio"
    }

    externalNativeBuild {
        cmake {
            // todo: need to disable REVERT for fast audio recording
            path = file("src/main/cpp/CMakeLists.txt")
        }
    }

    buildFeatures {
        buildConfig = true
        prefab = true
    }
}

dependencies {
    implementation(project(":base"))
    implementation(libs.appcompat)
}
