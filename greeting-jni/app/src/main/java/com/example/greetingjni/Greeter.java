/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package com.example.greetingjni;

public class Greeter {

  public String sendName(String name) {
    return stringFromJNI(name);
  }

  public void callUnimplementedJniMethod() {
    unimplementedStringFromJNI();
  }


  /* A native method that is implemented by the
    * 'greeting-jni' native library, which is packaged
    * with this application.
    */
  protected native String  stringFromJNI(String parameter);

  /* This is another native method declaration that is *not*
     * implemented by 'greeting-jni'. This is simply to show that
     * you can declare as many native methods in your Java code
     * as you want, their implementation is searched in the
     * currently loaded native libraries only the first time
     * you call them.
     *
     * Trying to call this function will result in a
     * java.lang.UnsatisfiedLinkError exception !
     */
  protected native String  unimplementedStringFromJNI();

  /* this is used to load the 'greeting-jni' library on application
   * startup. The library has already been unpacked into
   * /data/data/com.example.greeting-jni/lib/libgreeting-jni.so at
   * installation time by the package manager.
   */
  static {
    initLibrary();
  }

  /*
   * Using this technique to encapsulate the static loadLibrary block
   * to be able to UnitTest it.
   */
  static void initLibrary() {
    System.loadLibrary("greeting-jni");
  }
}
