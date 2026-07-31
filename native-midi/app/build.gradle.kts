plugins {
    id("ndksamples.android.application")
}

android {
    namespace = "com.example.nativemidi"

    defaultConfig {
        applicationId = "com.example.nativemidi"
        versionCode = 1
        versionName = "1.0"
        // 'AMidiOutputPort_receive' is unavailable: introduced in Android 29 android
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
