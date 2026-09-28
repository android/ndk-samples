plugins {
    id("ndksamples.android.application")
}

// Caution: If you add any Java or Kotlin code to this project, or if you add
// any dependencies which themselves use Java/Kotlin, you'll need to remove
// `android:hasCode` from main/AndroidManifest.xml.

android {
    namespace = "com.example.native_activity"

    defaultConfig {
        applicationId = "com.example.native_activity"
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
