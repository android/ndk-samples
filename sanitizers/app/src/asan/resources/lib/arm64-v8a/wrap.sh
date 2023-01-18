#!/system/bin/sh
# Copied from https://android.googlesource.com/platform/ndk/+/refs/heads/master/wrap.sh/asan.sh
HERE=$(cd "$(dirname "$0")" && pwd)

cmd=$1
shift

# This must be called *before* `LD_PRELOAD` is set. Otherwise, if this is a 32-
# bit app running on a 64-bit device, the 64-bit getprop will fail to load
# because it will preload a 32-bit ASan runtime.
# https://github.com/android/ndk/issues/1744
os_version=$(getprop ro.build.version.sdk)

if [ "$os_version" -eq "27" ]; then
  cmd="$cmd -Xrunjdwp:transport=dt_android_adb,suspend=n,server=y -Xcompiler-option --debuggable $@"
elif [ "$os_version" -eq "28" ]; then
  cmd="$cmd -XjdwpProvider:adbconnection -XjdwpOptions:suspend=n,server=y -Xcompiler-option --debuggable $@"
else
  cmd="$cmd -XjdwpProvider:adbconnection -XjdwpOptions:suspend=n,server=y $@"
fi

export ASAN_OPTIONS=log_to_syslog=false,allow_user_segv_handler=1
ASAN_LIB=$(ls "$HERE"/libclang_rt.asan-*-android.so)
if [ -f "$HERE/libc++_shared.so" ]; then
    # Workaround for https://github.com/android-ndk/ndk/issues/988.
    export LD_PRELOAD="$ASAN_LIB $HERE/libc++_shared.so"
else
    export LD_PRELOAD="$ASAN_LIB"
fi

exec $cmd