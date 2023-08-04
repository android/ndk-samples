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
1. Run the app on Android Studio. You can either run it on a physical or virtual
   device. You will see "Hello World" on the screen.
1. To pull the data from the device, you'll need to move it from an app-writable
   directory to a shell readable directory for adb pull. We also need to
   transfer the output into hexadecimal format.

```
adb shell "run-as com.example.orderfiledemo sh -c 'cat /data/user/0/com.example.orderfiledemo/cache/demo.output.order' | cat > /data/local/tmp/demo.output.order"
adb pull /data/local/tmp/demo.output.order .

# Convert to hexdeciaml format on Linux, Mac, or ChromeOS
hexdump -C demo.output.order > demo.prof

# Convert to hexdecimal format on Windows
certutil -f -encodeHex demo.output.order demo.prof
```

4. Once you get both mapping file and profile file, you can use
   [this script](https://android.googlesource.com/toolchain/pgo-profiles/+/refs/heads/main/scripts/create_orderfile.py)
   to create the order file:

```
python3 create_orderfile.py --profile-file demo.prof --mapping-file mapping.txt --output app/src/main/cpp/demo.orderfile
```

## Load Steps

1. For load, you need to uncomment
   `set(USE_PROFILE "${CMAKE_SOURCE_DIR}/demo.orderfile")` and make sure
   `set(GENERATE_PROFILES ON)` is commented.

1. If you want to validate the shared library's layout is different, you need to
   find `liborderfiledemo.so` and run `nm`

```
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
