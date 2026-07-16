plugins {
    id("ndksamples.android.application")
}

android {
    namespace = "com.sample.textureview"
    defaultConfig {
        applicationId = "com.sample.camera.textureview"
        minSdk = 24
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
    implementation(libs.androidx.core)
    implementation(project(":base"))
    implementation(project(":camera:camera-utils"))
}
