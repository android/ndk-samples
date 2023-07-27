Order file demo
============================

Order files are text files containing symbols representing functions names. Linkers (lld) uses order files to layout functions in a specific order. These binaries with ordered symbols will reduce page faults and improve a program's launch time due to the efficient loading of symbols during a program’s cold-start.

This demo was created based on the [PGO end-to-end demo](https://github.com/DanAlbert/ndk-samples/tree/pgo/pgo).

Files
--------
- app/src/main/cpp/orderfile.cpp: The source code for the orderfile library that is used by the Kotlin app. 
- app/src/main/cpp/CMakeLists.txt: The CMakeLists either sets the orderfile library as generating profiles or loading the orderfile.
- app/src/main/java/MainActivity.kt: The Kotlin app source code.

Profile Steps
---------------
1. For simplicity, we have setup the `CMakeLists.txt` and you just need make sure `set(GENERATE_PROFILES ON)` is not commented. You will notice it passes an optimization flag `-O1`. The mapping file is not generated and the profile instrumentation does not work without an optimization flag. 
2. Run the app on Android Studio. You can either run it on a physical or virtual device. You will see a "Hello World" on the screen.
3. To pull the data from the device, you'll need to move it from an app-writable directory to a shell readable directory for adb pull. This was take from the [PGO demo](https://github.com/DanAlbert/ndk-samples/tree/pgo/pgo). Unlike PGO, the output file is `demo.output.order` and we need to transfer the output into hexadecimal format.
```
adb shell "run-as com.example.orderfiledemo sh -c 'cat /data/user/0/com.example.orderfiledemo/cache/demo.output.order' | cat > /data/local/tmp/demo.output.order"
adb pull /data/local/tmp/demo.output.order .
od -h demo.output.order > demo.prof
```
4. Once you get both mapping file and profile file, you can use [this script](https://android.googlesource.com/toolchain/pgo-profiles/+/refs/heads/main/scripts/create_orderfile.py) to create the orderfile:
```
python3 create_orderfile.py --profile-file demo.prof --mapping-file mapping.txt --output app/src/main/cpp/demo.orderfile
```

Load Steps
---------------
1. For load, you need to uncomment `set(USE_PROFILE "${CMAKE_SOURCE_DIR}/demo.orderfile")` and make sure `set(GENERATE_PROFILES ON)` is commented.

2. If you want to validate the shared library's layout is different, you need to find `liborderfiledemo.so` and run `nm`
```
nm -n liborderfiledemo.so
```

License
--------
Copyright 2023 Google, Inc.

Licensed to the Apache Software Foundation (ASF) under one or more contributor license agreements. See the NOTICE file distributed with this work for additional information regarding copyright ownership. The ASF licenses this file to you under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.