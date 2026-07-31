plugins {
    id("ndksamples.android.application")
}

android {
    namespace = "com.android.accelerometergraph"

    defaultConfig {
        applicationId = "com.android.accelerometergraph"
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
