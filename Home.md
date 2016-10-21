# Requirements
* Android NDK r12+ for Android Studio + cmake/ndkbuild support
* For NDK r11 and earlier, gradle-experimental works, or other tools like ant. Branch android-mk keeps command-line / eclipse samples 

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
## How to Pack standard stl shared libs:
 Please check the [workaround](http://stackoverflow.com/questions/39620739)