/*
 * Copyright 2015 The Android Open Source Project
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

#include <stdint.h>
#include "check_permission.h"
#include "android_debug.h"

/*
 * This method checks the permission runtime permission specified, and then
 * invokes the callback with the results.  This callback may happen immediately if the permission
 * state is already known, or it may need to go through the requesting permission and onRequestPermissionsResult
 * before the callback is called.
 * <p>If there is already a pending permission check, the callback is invoked immediately  with
 * and error.</p>
 * @param permission - the permission to check.
 * @param rationale - the reason displayed to the user on why the permission is requested.
 * @param callbackPtr - the pointer to the callback function the result
 *                    value is either Packagemanager.PERMISSION_GRANTED (0),
 *                    Packagemanager.PERMISSION_DENIED (-1), or another negative number indicating
 *                    an error state.
 */
void checkPermission(JNIEnv *env, const char* permission, const char* rationale, void (*callback)(int result)) {
    jclass clz = env->FindClass("com/google/sample/echo/PermissionRequestFragment");
    if (clz) {
        jmethodID  method = env->GetStaticMethodID(clz,"checkPermission","(Ljava/lang/String;Ljava/lang/String;J)V");
        if (method) {
            jstring jPermission = env->NewStringUTF(permission);
            jstring jRationale = env->NewStringUTF(rationale);
            env->CallStaticVoidMethod(clz, method, jPermission, jRationale, callback);
            env->DeleteLocalRef(jPermission);
            env->DeleteLocalRef(jRationale);
        }
    }
}

/*
 * JNI entry point for Java calling back into native code with the result of the permission check.
 * @param env - the JNI environment
 * @param type - the class this method is associated with
 * @result - the result of the permission check.  The value is either Packagemanager.PERMISSION_GRANTED (0),
 *                    Packagemanager.PERMISSION_DENIED (-1), or another negative number indicating
 *                    an error state.
 * @callbackPtr - the function pointer passed in to checkPermission to receive the result.
 */
JNIEXPORT void JNICALL
Java_com_google_sample_echo_PermissionRequestFragment_handlePermissionResult(JNIEnv *env,
                                                                             jclass type,
                                                                             jint result,
                                                                             jlong callbackPtr) {
    if(callbackPtr) {
        void (*callbackfcn)(int) = (void(*)(int))callbackPtr;

        callbackfcn(static_cast<uint32_t>(result));
    } else {
        LOGE("** callback pointer is null!!");
    }
}
