/*
 * Copyright 2013 The Android Open Source Project
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

#pragma once

#include <jni.h>
#include <vector>
#include <string>

#include <android/log.h>
#include <android_native_app_glue.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, ndk_helper::JNIHelper::GetInstance()->GetAppName(), __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, ndk_helper::JNIHelper::GetInstance()->GetAppName(), __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, ndk_helper::JNIHelper::GetInstance()->GetAppName(), __VA_ARGS__))

namespace ndk_helper
{

/******************************************************************
 * Helper functions for JNI calls
 * This class wraps JNI calls and provides handy interface calling commonly used features
 * in Java SDK.
 * Such as
 * - loading graphics files (e.g. PNG, JPG)
 * - character code conversion
 * - retrieving system properties which only supported in Java SDK
 *
 * NOTE: To use this class, add NDKHelper.java as a corresponding helpers in Java code
 */
class JNIHelper
{
private:
    std::string app_name_;

    ANativeActivity* activity_;
    jobject jni_helper_java_ref_;
    jclass jni_helper_java_class_;

    //mutex for synchronization
    //This class uses singleton pattern and can be invoked from multiple threads,
    //each methods locks the mutex for a thread safety
    mutable pthread_mutex_t mutex_;

    jstring GetExternalFilesDirJString( JNIEnv *env );
    jclass RetrieveClass( JNIEnv *jni,
            const char* class_name );

    JNIHelper();
    ~JNIHelper();
    JNIHelper( const JNIHelper& rhs );
    JNIHelper& operator=( const JNIHelper& rhs );

public:
    /*
     * To load your own Java classes, JNIHelper requires to be initialized with a ANativeActivity handle.
     * This methods need to be called before any call to the helper class.
     * Static member of the class
     *
     * arguments:
     * in: activity, pointer to ANativeActivity. Used internally to set up JNI environment
     * in: helper_class_name, pointer to Java side helper class name. (e.g. "com/sample/helper/NDKHelper" in samples )
     */
    static void Init( ANativeActivity* activity,
            const char* helper_class_name );

    /*
     * Retrieve the singleton object of the helper.
     * Static member of the class

     * Methods in the class are designed as thread safe.
     */
    static JNIHelper* GetInstance();

    /*
     * Read a file from a strorage.
     * First, the method tries to read the file from an external storage.
     * If it fails to read, it falls back to use assset manager and try to read the file from APK asset.
     *
     * arguments:
     * in: file_name, file name to read
     * out: buffer_ref, pointer to a vector buffer to read a file.
     *      when the call succeeded, the buffer includes contents of specified file
     *      when the call failed, contents of the buffer remains same
     * return:
     * true when file read succeeded
     * false when it failed to read the file
     */
    bool ReadFile( const char* file_name,
            std::vector<uint8_t>* buffer_ref );

    /*
     * Load and create OpenGL texture from given file name.
     * The method invokes BitmapFactory in Java so it can read jpeg/png formatted files
     *
     * The methods creates mip-map and set texture parameters like this,
     * glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST );
     * glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
     * glGenerateMipmap( GL_TEXTURE_2D );
     *
     * arguments:
     * in: file_name, file name to read, PNG&JPG is supported
     * return:
     * OpenGL texture name when the call succeeded
     * When it failed to load the texture, it returns -1
     */
    uint32_t LoadTexture( const char* file_name );

    /*
     * Convert string from character code other than UTF-8
     *
     * arguments:
     *  in: str, pointer to a string which is encoded other than UTF-8
     *  in: encoding, pointer to a character encoding string.
     *  The encoding string can be any valid java.nio.charset.Charset name
     *  e.g. "UTF-16", "Shift_JIS"
     * return: converted input string as an UTF-8 std::string
     */
    std::string ConvertString( const char* str,
            const char* encode );
    /*
     * Retrieve external file directory through JNI call
     *
     * return: std::string containing external file diretory
     */
    std::string GetExternalFilesDir();

    /*
     * Audio helper
     * Retrieves native audio buffer size which is required to achieve low latency audio
     *
     * return: Native audio buffer size which is a hint to achieve low latency audio
     * If the API is not supported (API level < 17), it returns 0
     */
    int32_t GetNativeAudioBufferSize();

    /*
     * Audio helper
     * Retrieves native audio sample rate which is required to achieve low latency audio
     *
     * return: Native audio sample rate which is a hint to achieve low latency audio
     */
    int32_t GetNativeAudioSampleRate();

    /*
     * Retrieves application bundle name
     *
     * return: pointer to an app name string
     *
     */
    const char* GetAppName()
    {
        return app_name_.c_str();
    }

};

} //namespace ndkHelper
