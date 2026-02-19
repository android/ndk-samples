# Order file demo

Order files are text files containing symbols representing functions names.
Linkers (lld) uses order files to layout functions in a specific order. These
binaries with ordered symbols will reduce page faults and improve a program's
launch time due to the efficient loading of symbols during a program’s
cold-start.

## Files

- app/src/main/cpp/orderfile.cpp: The source code for the orderfile library that
  is used by the Kotlin app.
- app/src/main/cpp/CMakeLists.txt: The CMakeLists either sets the orderfile
  library as generating profiles or loading the orderfile.
- app/src/main/java/MainActivity.kt: The Kotlin app source code.

## Profile Steps

1. For simplicity, we have setup the `CMakeLists.txt` and you just need make
   sure `set(GENERATE_PROFILES ON)` is not commented. You need to pass any
   optimization flag except `-O0`. The mapping file is not generated and the
   profile instrumentation does not work without an optimization flag.
2. Run the app on Android Studio. You can either run it on a physical or virtual
   device. You will see "Hello World" on the screen.
3. To pull the data from the device, you'll need to move it from an app-writable
   directory to a shell readable directory for adb pull.
4. Use `llvm-profdata` to merge all the raw files and create an orderfile.

```
adb shell "run-as com.example.orderfiledemo sh -c 'cat /data/user/0/com.example.orderfiledemo/cache/demo.profraw' | cat > /data/local/tmp/demo.profraw"
adb pull /data/local/tmp/demo.profraw .
<NDK_PATH>/toolchains/llvm/prebuilt/<ARCH>/bin/llvm-profdata merge demo.profraw -o demo.profdata 
<NDK_PATH>/toolchains/llvm/prebuilt/<ARCH>/bin/llvm-profdata order demo.profdata -o demo.orderfile
```

## Load Steps

1. For load, you need to uncomment
   `set(USE_PROFILE "${CMAKE_SOURCE_DIR}/demo.orderfile")` and make sure
   `set(GENERATE_PROFILES ON)` is commented.

2. If you want to validate the shared library's layout is different, you need to
   find `liborderfiledemo.so` and run `nm`

```
mv demo.orderfile app/src/main/cpp
nm -n liborderfiledemo.so
```

## Difference between Java and Kotlin App

The main difference between a Java app and a Kotlin app is the syntax. You can
easily change this Kotlin example into a Java example.

- Load Library

```
# Kotlin
companion object {
    init {
        System.loadLibrary("orderfiledemo")
    }
}

# Java
static {
    System.loadLibrary("orderfiledemo");
}
```

- Recognize an external method

```
# Kotlin
external fun runWorkload(tempDir: String)

# Java
private native void runWorkload(String tempDir);
```

- Get the cache directory

```agsl
# Kotlin
runWorkload(applicationContext.cacheDir.toString())

# Java
runWorkload(getcacheDir().toString())
```
