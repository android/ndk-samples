Hello VK
=========
![Vulkan Triangle example](documentation-assets/example-triangle.png)
Hello VK is an Android C++ sample that draws the simple, but traditional Hello World Triangle.

Aside from the base functionality the source code also covers convenient features such as:

- Vulkan validation layers. See section below for information on how to enable these
- Vulkan pre-rotation 
to enable fast and seamless screen rotation implemented on the application side. You can read more about it here:
https://developer.android.com/games/optimize/vulkan-prerotation

Validation layer
-------
As the validation layer is a sizeable download, we chose to not ship them within the apk. As such in order to enable validation layer, please follow the simple steps below:

1. Download the latest android binaries from:
https://github.com/KhronosGroup/Vulkan-ValidationLayers/releases
2. Place them in their respective ABI folders located in: hellovk/app/src/main/jniLibs
3. Go to hellovk.h, search for 'bool enableValidationLayers = false' and toggle that to true. 

Extra information:
-------
You will notice the application uses VK_PRESENT_MODE_FIFO_KHR which is equivalent to VSYNC. This is done to simplify the presentation logic, 
but Vulkan allows you to choose different present modes depending on the properties of your mobile GPU. For a comprehensive reference, please check
https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPresentModeKHR.html

As Vulkan is well documented we will not provide detailed instructions regarding the innerworkings of Vulkan. You should however, find useful comments and references regarding the android to vulkan bridge. We chose to use Android Glue for a seamless experience. More details here:
https://developer.android.com/reference/games/game-activity/group/android-native-app-glue

These vulkan tutorials should hopefully cover everything needed to understand the workings of the Vulkan app:
https://vulkan-tutorial.com
https://vkguide.dev/docs/chapter-1/vulkan_init_flow/

Lastly, you will notice that the Kotlin file is somewhat redundant. Luckily, if you do not require any additional/custom application behaviour, the Android(Kotlin) source files can be completely removed and the AndroidManifest.xml tweaked as specified here:
https://developer.android.com/ndk/samples/sample_na


License
-------

  Copyright (C) 2022 The Android Open Source Project
 
  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the 
  
  License at
  http://www.apache.org/licenses/LICENSE-2.0
 
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
 