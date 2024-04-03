# Hello VK

Hello VK is an Android C++ sample that draws the simple, but traditional Hello
World Triangle.

Aside from the base functionality the source code also covers convenient
features such as:

- Vulkan validation layers. See section below for information on how to enable
  these
- Vulkan pre-rotation to enable fast and seamless screen rotation implemented on
  the application side. You can read more about it here:
  https://developer.android.com/games/optimize/vulkan-prerotation

## Pre-requisites

- Android Studio 4.2+ with [NDK](https://developer.android.com/ndk/) bundle.

## Getting Started

1. [Download Android Studio](http://developer.android.com/sdk/index.html)
1. Launch Android Studio.
1. Open the sample directory.
1. Open *File/Project Structure...*

- Click *Download* or *Select NDK location*.

1. Click *File/Sync Project with Gradle Files*.
1. Click *Run/Run 'app'*.

## Screenshots

![Vulkan Triangle example](documentation-assets/example-triangle.png)

## Validation layers

As the validation layer is a sizeable download, we chose to not ship them within
the apk. As such in order to enable validation layer, please follow the simple
steps below:

1. Download the latest android binaries from:
   https://github.com/KhronosGroup/Vulkan-ValidationLayers/releases
1. Place them in their respective ABI folders located in: app/src/main/jniLibs
1. Go to hellovk.h, search for 'bool enableValidationLayers = false' and toggle
   that to true.

## Extra information:

As Vulkan is well documented we will not provide detailed instructions regarding
the innerworkings of Vulkan. You should however, find useful comments and
references regarding the android to vulkan bridge. We chose to use Android Glue
for a seamless experience. More details here:
https://developer.android.com/reference/games/game-activity/group/android-native-app-glue

These vulkan tutorials should hopefully cover everything needed to understand
the workings of the Vulkan app: https://vulkan-tutorial.com
https://vkguide.dev/docs/chapter-1/vulkan_init_flow/

Lastly, you will notice that the Kotlin file is somewhat redundant. Luckily, if
you do not require any additional/custom application behaviour, the
Android(Kotlin) source files can be completely removed and the
AndroidManifest.xml tweaked as specified here:
https://developer.android.com/ndk/samples/sample_na

## Support

If you've found an error in these samples, please
[file an issue](https://github.com/googlesamples/android-ndk/issues/new).

Patches are encouraged, and may be submitted by
[forking this project](https://github.com/googlesamples/android-ndk/fork) and
submitting a pull request through GitHub. Please see
[CONTRIBUTING.md](../CONTRIBUTING.md) for more details.

- [Stack Overflow](http://stackoverflow.com/questions/tagged/android-ndk)
- [Android Tools Feedbacks](http://tools.android.com/feedback)
