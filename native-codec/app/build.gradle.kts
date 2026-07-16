plugins {
    id("ndksamples.android.application")
}

android {
    namespace = "com.example.nativecodec"

    defaultConfig {
        applicationId = "com.example.nativecodec"
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
}
