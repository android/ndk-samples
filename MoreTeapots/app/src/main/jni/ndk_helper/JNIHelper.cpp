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
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <fstream>
#include <iostream>

#include "JNIHelper.h"

namespace ndk_helper
{

#define CLASS_NAME "android/app/NativeActivity"

//---------------------------------------------------------------------------
//JNI Helper functions
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//Singleton
//---------------------------------------------------------------------------
JNIHelper* JNIHelper::GetInstance()
{
    static JNIHelper helper;
    return &helper;
}

//---------------------------------------------------------------------------
//Ctor
//---------------------------------------------------------------------------
JNIHelper::JNIHelper()
{
    pthread_mutex_init( &mutex_, NULL );
}

//---------------------------------------------------------------------------
//Dtor
//---------------------------------------------------------------------------
JNIHelper::~JNIHelper()
{
    pthread_mutex_lock( &mutex_ );

    JNIEnv *env;
    activity_->vm->AttachCurrentThread( &env, NULL );

    env->DeleteGlobalRef( jni_helper_java_ref_ );
    env->DeleteGlobalRef( jni_helper_java_class_ );

    activity_->vm->DetachCurrentThread();

    pthread_mutex_destroy( &mutex_ );
}

//---------------------------------------------------------------------------
//Init
//---------------------------------------------------------------------------
void JNIHelper::Init( ANativeActivity* activity,
        const char* helper_class_name )
{
    JNIHelper& helper = *GetInstance();
    pthread_mutex_lock( &helper.mutex_ );

    helper.activity_ = activity;

    JNIEnv *env;
    helper.activity_->vm->AttachCurrentThread( &env, NULL );

    //Retrieve app name
    jclass android_content_Context = env->GetObjectClass( helper.activity_->clazz );
    jmethodID midGetPackageName = env->GetMethodID( android_content_Context, "getPackageName",
            "()Ljava/lang/String;" );

    jstring packageName = (jstring) env->CallObjectMethod( helper.activity_->clazz,
            midGetPackageName );
    const char* appname = env->GetStringUTFChars( packageName, NULL );
    helper.app_name_ = std::string( appname );

    jclass cls = helper.RetrieveClass( env, helper_class_name );
    helper.jni_helper_java_class_ = (jclass) env->NewGlobalRef( cls );

    jmethodID constructor = env->GetMethodID( helper.jni_helper_java_class_, "<init>", "()V" );
    helper.jni_helper_java_ref_ = env->NewObject( helper.jni_helper_java_class_, constructor );
    helper.jni_helper_java_ref_ = env->NewGlobalRef( helper.jni_helper_java_ref_ );

    env->ReleaseStringUTFChars( packageName, appname );
    helper.activity_->vm->DetachCurrentThread();

    pthread_mutex_unlock( &helper.mutex_ );
}

//---------------------------------------------------------------------------
//readFile
//---------------------------------------------------------------------------
bool JNIHelper::ReadFile( const char* fileName,
        std::vector<uint8_t>* buffer_ref )
{
    if( activity_ == NULL )
    {
        LOGI( "JNIHelper has not been initialized.Call init() to initialize the helper" );
        return false;
    }

    //First, try reading from externalFileDir;
    JNIEnv *env;
    jmethodID mid;

    pthread_mutex_lock( &mutex_ );
    activity_->vm->AttachCurrentThread( &env, NULL );

    jstring str_path = GetExternalFilesDirJString( env );
    const char* path = env->GetStringUTFChars( str_path, NULL );
    std::string s( path );

    if( fileName[0] != '/' )
    {
        s.append( "/" );
    }
    s.append( fileName );
    std::ifstream f( s.c_str(), std::ios::binary );

    env->ReleaseStringUTFChars( str_path, path );
    env->DeleteLocalRef( str_path );
    activity_->vm->DetachCurrentThread();

    if( f )
    {
        LOGI( "reading:%s", s.c_str() );
        f.seekg( 0, std::ifstream::end );
        int32_t fileSize = f.tellg();
        f.seekg( 0, std::ifstream::beg );
        buffer_ref->reserve( fileSize );
        buffer_ref->assign( std::istreambuf_iterator<char>( f ), std::istreambuf_iterator<char>() );
        f.close();
        pthread_mutex_unlock( &mutex_ );
        return true;
    }
    else
    {
        //Fallback to assetManager
        AAssetManager* assetManager = activity_->assetManager;
        AAsset* assetFile = AAssetManager_open( assetManager, fileName, AASSET_MODE_BUFFER );
        if( !assetFile )
        {
            pthread_mutex_unlock( &mutex_ );
            return false;
        }
        uint8_t* data = (uint8_t*) AAsset_getBuffer( assetFile );
        int32_t size = AAsset_getLength( assetFile );
        if( data == NULL )
        {
            AAsset_close( assetFile );

            LOGI( "Failed to load:%s", fileName );
            pthread_mutex_unlock( &mutex_ );
            return false;
        }

        buffer_ref->reserve( size );
        buffer_ref->assign( data, data + size );

        AAsset_close( assetFile );
        pthread_mutex_unlock( &mutex_ );
        return true;
    }
}

std::string JNIHelper::GetExternalFilesDir()
{
    if( activity_ == NULL )
    {
        LOGI( "JNIHelper has not been initialized. Call init() to initialize the helper" );
        return std::string( "" );
    }

    pthread_mutex_lock( &mutex_ );

    //First, try reading from externalFileDir;
    JNIEnv *env;
    jmethodID mid;

    activity_->vm->AttachCurrentThread( &env, NULL );

    jstring strPath = GetExternalFilesDirJString( env );
    const char* path = env->GetStringUTFChars( strPath, NULL );
    std::string s( path );

    env->ReleaseStringUTFChars( strPath, path );
    env->DeleteLocalRef( strPath );
    activity_->vm->DetachCurrentThread();

    pthread_mutex_unlock( &mutex_ );
    return s;
}

uint32_t JNIHelper::LoadTexture( const char* file_name )
{
    if( activity_ == NULL )
    {
        LOGI( "JNIHelper has not been initialized. Call init() to initialize the helper" );
        return 0;
    }

    JNIEnv *env;
    jmethodID mid;

    pthread_mutex_lock( &mutex_ );
    activity_->vm->AttachCurrentThread( &env, NULL );

    jstring name = env->NewStringUTF( file_name );

    GLuint tex;
    glGenTextures( 1, &tex );
    glBindTexture( GL_TEXTURE_2D, tex );

    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    mid = env->GetMethodID( jni_helper_java_class_, "loadTexture", "(Ljava/lang/String;)Z" );
    jboolean ret = env->CallBooleanMethod( jni_helper_java_ref_, mid, name );
    if( !ret )
    {
        glDeleteTextures( 1, &tex );
        tex = -1;
        LOGI( "Texture load failed %s", file_name );
    }

    //Generate mipmap
    glGenerateMipmap( GL_TEXTURE_2D );

    env->DeleteLocalRef( name );
    activity_->vm->DetachCurrentThread();
    pthread_mutex_unlock( &mutex_ );

    return tex;

}

std::string JNIHelper::ConvertString( const char* str,
        const char* encode )
{
    if( activity_ == NULL )
    {
        LOGI( "JNIHelper has not been initialized. Call init() to initialize the helper" );
        return std::string( "" );
    }

    JNIEnv *env;

    pthread_mutex_lock( &mutex_ );
    activity_->vm->AttachCurrentThread( &env, NULL );

    int32_t iLength = strlen( (const char*) str );

    jbyteArray array = env->NewByteArray( iLength );
    env->SetByteArrayRegion( array, 0, iLength, (const signed char*) str );

    jstring strEncode = env->NewStringUTF( encode );

    jclass cls = env->FindClass( "java/lang/String" );
    jmethodID ctor = env->GetMethodID( cls, "<init>", "([BLjava/lang/String;)V" );
    jstring object = (jstring) env->NewObject( cls, ctor, array, strEncode );

    const char *cparam = env->GetStringUTFChars( object, NULL );

    std::string s = std::string( cparam );

    env->ReleaseStringUTFChars( object, cparam );
    env->DeleteLocalRef( strEncode );
    env->DeleteLocalRef( object );
    activity_->vm->DetachCurrentThread();
    pthread_mutex_unlock( &mutex_ );

    return s;
}

//---------------------------------------------------------------------------
//Audio helpers
//---------------------------------------------------------------------------
int32_t JNIHelper::GetNativeAudioBufferSize()
{
    if( activity_ == NULL )
    {
        LOGI( "JNIHelper has not been initialized. Call init() to initialize the helper" );
        return 0;
    }

    JNIEnv *env;
    jmethodID mid;

    pthread_mutex_lock( &mutex_ );
    activity_->vm->AttachCurrentThread( &env, NULL );

    mid = env->GetMethodID( jni_helper_java_class_, "getNativeAudioBufferSize", "()I" );
    int32_t i = env->CallIntMethod( jni_helper_java_ref_, mid );
    activity_->vm->DetachCurrentThread();
    pthread_mutex_unlock( &mutex_ );

    return i;
}

int32_t JNIHelper::GetNativeAudioSampleRate()
{
    if( activity_ == NULL )
    {
        LOGI( "JNIHelper has not been initialized. Call init() to initialize the helper" );
        return 0;
    }

    JNIEnv *env;
    jmethodID mid;

    pthread_mutex_lock( &mutex_ );
    activity_->vm->AttachCurrentThread( &env, NULL );

    mid = env->GetMethodID( jni_helper_java_class_, "getNativeAudioSampleRate", "()I" );
    int32_t i = env->CallIntMethod( jni_helper_java_ref_, mid );
    activity_->vm->DetachCurrentThread();
    pthread_mutex_unlock( &mutex_ );

    return i;
}

//---------------------------------------------------------------------------
//Misc implementations
//---------------------------------------------------------------------------
jclass JNIHelper::RetrieveClass( JNIEnv *jni,
        const char* class_name )
{
    jclass activity_class = jni->FindClass( CLASS_NAME );
    jmethodID get_class_loader = jni->GetMethodID( activity_class, "getClassLoader",
            "()Ljava/lang/ClassLoader;" );
    jobject cls = jni->CallObjectMethod( activity_->clazz, get_class_loader );
    jclass class_loader = jni->FindClass( "java/lang/ClassLoader" );
    jmethodID find_class = jni->GetMethodID( class_loader, "loadClass",
            "(Ljava/lang/String;)Ljava/lang/Class;" );

    jstring str_class_name = jni->NewStringUTF( class_name );
    jclass class_retrieved = (jclass) jni->CallObjectMethod( cls, find_class, str_class_name );
    jni->DeleteLocalRef( str_class_name );
    return class_retrieved;
}

jstring JNIHelper::GetExternalFilesDirJString( JNIEnv *env )
{
    if( activity_ == NULL )
    {
        LOGI( "JNIHelper has not been initialized. Call init() to initialize the helper" );
        return NULL;
    }

    // Invoking getExternalFilesDir() java API
    jclass cls_Env = env->FindClass( CLASS_NAME );
    jmethodID mid = env->GetMethodID( cls_Env, "getExternalFilesDir",
            "(Ljava/lang/String;)Ljava/io/File;" );
    jobject obj_File = env->CallObjectMethod( activity_->clazz, mid, NULL );
    jclass cls_File = env->FindClass( "java/io/File" );
    jmethodID mid_getPath = env->GetMethodID( cls_File, "getPath", "()Ljava/lang/String;" );
    jstring obj_Path = (jstring) env->CallObjectMethod( obj_File, mid_getPath );

    return obj_Path;
}

} //namespace ndkHelper
