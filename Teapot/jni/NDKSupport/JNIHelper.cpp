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

#include "JNIHelper.h"

//---------------------------------------------------------------------------
//JNI Helper functions
//---------------------------------------------------------------------------
//Static variable
ANativeActivity* JNIHelper::_activity;
jobject JNIHelper::_objJNIHelper;
jclass JNIHelper::_clsJNIHelper;

jclass retrieveClass(JNIEnv *jni, ANativeActivity* activity,
        const char* className) {
    jclass activityClass = jni->FindClass(CLASS_NAME);
    jmethodID getClassLoader = jni->GetMethodID(activityClass, "getClassLoader",
            "()Ljava/lang/ClassLoader;");
    jobject cls = jni->CallObjectMethod(activity->clazz, getClassLoader);
    jclass classLoader = jni->FindClass("java/lang/ClassLoader");
    jmethodID findClass = jni->GetMethodID(classLoader, "loadClass",
            "(Ljava/lang/String;)Ljava/lang/Class;");

    jstring strClassName = jni->NewStringUTF(className);
    jclass classRetrieved = (jclass) jni->CallObjectMethod(cls, findClass,
            strClassName);
    jni->DeleteLocalRef(strClassName);
    return classRetrieved;
}

//---------------------------------------------------------------------------
//readFile
//---------------------------------------------------------------------------
bool JNIHelper::readFile(const char* fileName, std::vector<uint8_t>& buffer) {
    if (_activity == NULL) {
        return false;
    }

    //First, try reading from externalFileDir;
    JNIEnv *env;
    jmethodID mid;

    _activity->vm->AttachCurrentThread(&env, NULL);

    jstring strPath = getExternalFilesDir(env);
    const char* path = env->GetStringUTFChars(strPath, NULL);
    std::string s(path);

    if (fileName[0] != '/') {
        s.append("/");
    }
    s.append(fileName);
    std::ifstream f(s.c_str(), std::ios::binary);

    env->ReleaseStringUTFChars(strPath, path);
    _activity->vm->DetachCurrentThread();

    if (f) {
        LOGI("reading:%s", s.c_str());
        f.seekg(0, std::ifstream::end);
        int32_t fileSize = f.tellg();
        f.seekg(0, std::ifstream::beg);
        buffer.reserve(fileSize);
        buffer.assign(std::istreambuf_iterator<char>(f),
                std::istreambuf_iterator<char>());
        return true;
    } else {
        //Fallback to assetManager
        AAssetManager* assetManager = _activity->assetManager;
        AAsset* assetFile = AAssetManager_open(assetManager, fileName,
                AASSET_MODE_BUFFER);
        if (!assetFile) {
            return false;
        }
        uint8_t* data = (uint8_t*) AAsset_getBuffer(assetFile);
        int32_t iSize = AAsset_getLength(assetFile);
        if (data == NULL) {
            AAsset_close(assetFile);

            LOGI("Failed to load:%s", fileName);
            return false;
        }

        buffer.assign(data, data + iSize);

        AAsset_close(assetFile);
        return true;
    }
}

jstring JNIHelper::getExternalFilesDir(JNIEnv *env) {
    if (_activity == NULL) {
        return NULL;
    }

    // getExternalFilesDir() - java
    jclass cls_Env = env->FindClass("android/app/NativeActivity");
    jmethodID mid = env->GetMethodID(cls_Env, "getExternalFilesDir",
            "(Ljava/lang/String;)Ljava/io/File;");
    jobject obj_File = env->CallObjectMethod(_activity->clazz, mid, NULL);
    jclass cls_File = env->FindClass("java/io/File");
    jmethodID mid_getPath = env->GetMethodID(cls_File, "getPath",
            "()Ljava/lang/String;");
    jstring obj_Path = (jstring) env->CallObjectMethod(obj_File, mid_getPath);
    return obj_Path;
}

uint32_t JNIHelper::loadTexture(const char* fileName) {
    if (_activity == NULL) {
        return 0;
    }

    JNIEnv *env;
    jmethodID mid;

    _activity->vm->AttachCurrentThread(&env, NULL);

    if (_clsJNIHelper == 0) {
        jclass cls = retrieveClass(env, _activity, APPLICATION_CLASS_NAME);
        _clsJNIHelper = (jclass) env->NewGlobalRef(cls);

        jmethodID constructor = env->GetMethodID(_clsJNIHelper, "<init>",
                "()V");
        _objJNIHelper = env->NewObject(_clsJNIHelper, constructor);
        _objJNIHelper = env->NewGlobalRef(_objJNIHelper);
    }

    /* Ask the PNG manager for a bitmap */
    mid = env->GetMethodID(_clsJNIHelper, "openBitmap",
            "(Ljava/lang/String;Z)Landroid/graphics/Bitmap;");

    jstring name = env->NewStringUTF(fileName);
#if 0
    jobject png = env->CallObjectMethod(_objJNIHelper, mid, name, true);
    env->DeleteLocalRef(name);
    env->NewGlobalRef(png);

    /* Get image dimensions */
    mid = env->GetMethodID(_clsJNIHelper, "getBitmapWidth", "(Landroid/graphics/Bitmap;)I");
    int width = env->CallIntMethod(_objJNIHelper, mid, png);
    mid = env->GetMethodID(_clsJNIHelper, "getBitmapHeight", "(Landroid/graphics/Bitmap;)I");
    int height = env->CallIntMethod(_objJNIHelper, mid, png);

    /* Get pixels */
    jintArray array = env->NewIntArray(width * height);
    env->NewGlobalRef(array);
    mid = env->GetMethodID(_clsJNIHelper, "getBitmapPixels", "(Landroid/graphics/Bitmap;[I)V");
    env->CallVoidMethod(_objJNIHelper, mid, png, array);

    jint *pixels = env->GetIntArrayElements(array, 0);

    LOGI( "Loaded bitmap %s, width %d height %d",fileName, width, height);

    GLuint tex;
    glGenTextures( 1, &tex );
    glBindTexture( GL_TEXTURE_2D, tex );

    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
            GL_LINEAR_MIPMAP_NEAREST );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    //Generate mipmap
    glGenerateMipmap(GL_TEXTURE_2D);

    env->ReleaseIntArrayElements(array, pixels, 0);
    env->DeleteGlobalRef(array);

    /* Free image */
    mid = env->GetMethodID(_clsJNIHelper, "closeBitmap", "(Landroid/graphics/Bitmap;)V");
    env->CallVoidMethod(_objJNIHelper, mid, png);
    env->DeleteGlobalRef(png);
#else
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
            GL_LINEAR_MIPMAP_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    mid = env->GetMethodID(_clsJNIHelper, "loadTexture",
            "(Ljava/lang/String;)V");
    env->CallVoidMethod(_objJNIHelper, mid, name);

    //Generate mipmap
    glGenerateMipmap(GL_TEXTURE_2D);

#endif

    _activity->vm->DetachCurrentThread();

    return tex;

}

