#include <errno.h>
#include <linux/limits.h>
#include <stdlib.h>
#include <string.h>

#include <jni.h>

#include <android/log.h>

const char kLogTag[] = "pgodemo";

#ifdef GENERATE_PROFILES
extern "C" int __llvm_profile_set_filename(const char *);
extern "C" int __llvm_profile_write_file(void);
#endif

void DumpProfileDataIfNeeded(const char* temp_dir) {
#ifdef GENERATE_PROFILES
    char profile_location[PATH_MAX] = {};
    snprintf(profile_location, sizeof(profile_location), "%s/default.profraw", temp_dir);
    if (__llvm_profile_set_filename(profile_location) == -1) {
        __android_log_print(
                ANDROID_LOG_ERROR,
                kLogTag,
                "__llvm_profile_set_filename(\"%s\") failed: %s",
                profile_location,
                strerror(errno));
        return;
    }
    if (__llvm_profile_write_file() == -1) {
        __android_log_print(
                ANDROID_LOG_ERROR,
                kLogTag,
                "__llvm_profile_write_file() failed: %s",
                strerror(errno));
        return;
    }
    __android_log_print(
            ANDROID_LOG_DEBUG,
            kLogTag,
            "Wrote profile data to %s",
            profile_location);
#else
    __android_log_print(
            ANDROID_LOG_DEBUG, kLogTag,
            "Did not write profile data because the app was not built for profile generation");
#endif
}

int Add(int a, int b) {
    return a + b;
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_pgodemo_MainActivity_runWorkload(
        JNIEnv *env,
        jobject /* this */,
        jstring temp_dir) {
    Add(2, 2);
    DumpProfileDataIfNeeded(env->GetStringUTFChars(temp_dir, 0));
}