plugins {
    id("ndksamples.android.application")
}

android {
    namespace = "com.android.gles3jni"

    defaultConfig {
        applicationId = "com.android.gles3jni"
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
