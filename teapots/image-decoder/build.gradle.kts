plugins {
    id("ndksamples.android.application")
}

android {
    namespace = "com.sample.imagedecoder"

    defaultConfig {
        applicationId = "com.sample.imagedecoder"
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
}

dependencies {
    implementation(libs.appcompat)
    implementation(libs.androidx.constraintlayout)
}
