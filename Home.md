# Generic Build Instructions
* Android Studio: use "Import project (Eclipse ADT, Gradle, etc)", or "File" > "Import Project", local.properties file will be created. Double check sdk and ndk directories are setting correctly in that file.
* Command Line: set up two environment variables ANDROID_HOME (for sdk dir) and ANDROID_NDK_HOME(for ndk dir), then "gradlew assembleDebug"


# Q and A
## How to trace code in native lib?   
* In app/build.gradle, add compile dependency for debug and release builds with:   
debugCompile project(path: ':your-lib', configuration: 'debug')   
releaseCompile project(path: ':your-lib', configuration: 'release')
* in lib(s)/build.gradle, enable publishNonDefault:    
```java
android {
    publishNonDefault  true
}
```
