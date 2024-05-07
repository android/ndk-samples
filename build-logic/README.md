# Convention plugins

This directory contains [convention plugins] used by the NDK samples. These are
used to remove Gradle boiler plate from individual samples in favor of common
configuration here. Using convention plugins for single module projects is
overkill, but any non-trivial app will likely need their own eventually. See
[Now In Android's build-logic][nia-build-logic] for a more thorough example of
building convention plugins for Android projects.

[convention plugins]: https://docs.gradle.org/current/samples/sample_convention_plugins.html
[nia-build-logic]: https://github.com/android/nowinandroid/blob/main/build-logic/README.md
