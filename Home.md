# Generic Build Instructions
* Android Studio: use "Import project (Eclipse ADT, Gradle, etc)", or "File" > "Import Project", local.properties file will be created. Double check sdk and ndk directories are setting correctly in that file.
* Command Line: set up two environment variables ANDROID_HOME (for sdk dir) and ANDROID_NDK_HOME(for ndk dir), then "gradlew assembleDebug"


