plugins {
    id("ndksamples.android.application")
}

android {
    namespace = "com.sample.camera.basic"
    defaultConfig {
        applicationId = "com.sample.camera.basic"
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
    implementation(project(":camera:camera-utils"))
}
