# Native Activity

> [!WARNING]
> **Most apps should not use the app development model shown in this sample**.
> Instead, use a Java or Kotlin `AppCompatActivity` and connect your native code
> using JNI like the other samples in this repository. `NativeActivity` and
> `GameActivity` attempt to translate the Android [activity lifecycle] into a
> desktop style `main()` function with a polled event loop. That is not how
> Android apps work, and while it may help you get your prototype running more
> quickly, as your app matures you will likely end up retranslating the
> `native_app_glue` model to again look like `Activity`.

This is an Android sample that uses [NativeActivity] with `native_app_glue`,
which enables building NDK apps without having to write any Java code. In
practice most apps, even games which are predominantly native code, will need to
call some Java APIs or customize their app's activity further. While it may save
you a small amount of effort during prototyping, it may result in a difficult
migration later. It's also worth noting that some of the code in this sample is
spent undoing the work of `native_app_glue` to create a class very similar to
`Activity`.

The more modern approach to this is to use [GameActivity], which has all the
same benefits as `NativeActivity` and `native_app_glue`, while also making it
easier to include Java code in your app without a rewrite later. It's also
source compatible. However, it still has all the problems explained in the
warning above, and in practice neither `NativeActivity` nor `GameActivity` is
the recommended app development model.

The app here is intentionally quite simple, aiming to show the core event and
draw loop necessary for an app using `native_app_glue` without any extra
clutter. It uses `AChoreographer` to manage the update/render loop, and uses
`ANativeWindow` and `AHardwareBuffer` to update the screen with a simple color
clear.

[activity lifecycle]: https://developer.android.com/guide/components/activities/activity-lifecycle
[GameActivity]: https://developer.android.com/games/agdk/game-activity
[NativeActivity]: http://developer.android.com/reference/android/app/NativeActivity.html

## Walkthrough

The interesting sections of code in this sample are in three files:
[AndroidManifest.xml], [CMakeLists.txt], and [main.cpp]. Each of those files has
code comments explaining the portions relevant to using `NativeActivity`, but
the high level details of the app are explained here.

This app uses `NativeActivity` rather than its own child class of `Activity` or
`AppCompatActivity`. This is specified in the `<activity />` declaration in [the
manifest].

Apps which use `NativeActivity` are typically written using `native_app_glue`,
which adapts the Android activity lifecycle code to look more like a desktop
program with a `main()` function and an event loop. This is set up in the app's
[CMakeLists.txt file].

When using `native_app_glue` with a [version script], you must export
`ANativeActivity_onCreate`. This sample does this in
[libnative-activity.map.txt].

This is a fairly simple application, so all of the code is in a single file,
[main.cpp]. The entry point for an app using `native_app_glue` is
`android_main()`. That function is the best place to start reading in this file
to learn how the sample works, then follow through to the definition of
`engine_handle_cmd` and `Engine`.

[CMakeLists.txt file]: app/src/main/cpp/CMakeLists.txt
[libnative-activity.map.txt]: app/src/main/cpp/libnative-activity.map.txt
[main.cpp]: app/src/main/cpp/main.cpp
[the manifest]: app/src/main/AndroidManifest.xml
[version script]: https://developer.android.com/ndk/guides/symbol-visibility
