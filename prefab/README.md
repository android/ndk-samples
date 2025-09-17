# Sample removed

The samples that used to reside in this directory have been removed. The
behaviors demonstrated here (publication and consumption of AARs with native
APIs) are now used throughout the samples for code sharing, and were Android
Gradle Plugin features rather than NDK features.

For an example of how to create an AAR that exposes a native library, see the
`base` module, or any other with `buildFeatures { prefabPublishing = true }`.

For examples of how to consume native APIs from an AAR, see any of the other
samples which set `buildFeatures { prefab = true }`.

See https://developer.android.com/build/native-dependencies for the Android
Gradle Plugin's documentation of these features.
