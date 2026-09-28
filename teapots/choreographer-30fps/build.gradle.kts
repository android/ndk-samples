plugins {
    id("ndksamples.android.application")
}

android {
    namespace = "com.sample.choreographer"

    defaultConfig {
        applicationId = "com.sample.choreographer"
        // 'AImageDecoder' is unavailable: introduced in Android 30 android
        minSdk = 30
        externalNativeBuild {
            cmake {
                arguments.add("-DANDROID_STL=c++_static")
            }
        }
    }

    externalNativeBuild {
        cmake {
            path = file("src/main/cpp/CMakeLists.txt")
        }
    }

    buildFeatures {
        prefab = true
    }
}

dependencies {
    implementation(project(":base"))
    implementation(libs.appcompat)
    implementation(libs.androidx.constraintlayout)
}
