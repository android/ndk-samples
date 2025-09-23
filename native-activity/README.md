# Native Activity

This is an Android sample that uses [NativeActivity] with `native_app_glue`,
which enables building NDK apps without having to write any Java code. In
practice most apps, even games which are predominantly native code, will need to
call some Java APIs or customize their app's activity further.

The more modern approach to this is to use [GameActivity], which has all the
same benefits as `NativeActivity` and `native_app_glue`, while also making it
easier to include Java code in your app without a rewrite later. It's also
source compatible. This sample will likely migrate to `GameActivity` in the
future.

The app here is intentionally quite simple, aiming to show the core event and
draw loop necessary for an app using `native_app_glue` without any extra
clutter. It uses `AChoreographer` to manage the update/render loop, and uses
`ANativeWindow` and `AHardwareBuffer` to update the screen with a simple color
clear.

[GameActivity]: https://developer.android.com/games/agdk/game-activity
[NativeActivity]: http://developer.android.com/reference/android/app/NativeActivity.html

## Screenshots

![screenshot](screenshot.png)
