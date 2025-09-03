plugins {
    id("ndksamples.android.library")
}

android {
    namespace = "com.android.ndk.samples.camera.utils"

    defaultConfig {
        minSdk = 24
    }

    externalNativeBuild {
        cmake {
            path = file("src/main/cpp/CMakeLists.txt")
        }
    }

    buildFeatures {
        prefabPublishing = true
    }

    prefab {
        create("camera-utils") {
            headers = "src/main/cpp/include"
        }
    }
}
