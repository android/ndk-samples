/*
 * Copyright (C) 2022 The Android Open Source Project
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

#include <android/asset_manager.h>
#include <android/log.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <assert.h>
#include <vulkan/vulkan.h>

#include <array>
#include <fstream>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <vector>

/**
 * HelloVK contains the core of Vulkan pipeline setup. It includes recording
 * draw commands as well as screen clearing during the render pass.
 *
 * Please refer to: https://vulkan-tutorial.com/ for a gentle Vulkan
 * introduction.
 */

namespace vkt {
#define LOG_TAG "hellovkjni"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define VK_CHECK(x)                           \
  do {                                        \
    VkResult err = x;                         \
    if (err) {                                \
      LOGE("Detected Vulkan error: %d", err); \
      abort();                                \
    }                                         \
  } while (0)

const int MAX_FRAMES_IN_FLIGHT = 2;

struct UniformBufferObject {
  std::array<float, 16> mvp;
};

struct QueueFamilyIndices {
  std::optional<uint32_t> graphicsFamily;
  std::optional<uint32_t> presentFamily;
  bool isComplete() {
    return graphicsFamily.has_value() && presentFamily.has_value();
  }
};

struct SwapChainSupportDetails {
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> presentModes;
};

struct ANativeWindowDeleter {
  void operator()(ANativeWindow *window) { ANativeWindow_release(window); }
};

class HelloVK {
 public:
  void initVulkan();
  void render();
  void cleanup();
  void cleanupSwapChain();
  void reset(ANativeWindow *newWindow, AAssetManager *newManager);
  bool initialized = false;

 private:
  void createInstance();
  void createSurface();
  void setupDebugMessenger();
  void pickPhysicalDevice();
  void createLogicalDeviceAndQueue();
  void createSwapChain();
  void createImageViews();
  void createRenderPass();
  void createDescriptorSetLayout();
  void createGraphicsPipeline();
  void createFrameBuffers();
  void createCommandPool();
  void createCommandBuffer();
  void createSyncObjects();
  QueueFamilyIndices findQueueFamilies(VkPhysicalDevice candidateDevice);
  bool checkDeviceExtensionSupport(VkPhysicalDevice candidateDevice);
  bool isDeviceSuitable(VkPhysicalDevice candidateDevice);
  bool checkValidationLayerSupport();
  std::vector<const char *> getRequiredExtensions();
  SwapChainSupportDetails querySwapChainSupport(
      VkPhysicalDevice candidateDevice);
  VkShaderModule createShaderModule(const std::vector<uint8_t> &code);
  void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
  void recreateSwapChain();
  void onOrientationChange();
  uint32_t findMemoryType(uint32_t typeFilter,
                          VkMemoryPropertyFlags properties);
  void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                    VkMemoryPropertyFlags properties, VkBuffer &buffer,
                    VkDeviceMemory &bufferMemory);
  void createUniformBuffers();
  void updateUniformBuffer(uint32_t currentImage);
  void createDescriptorPool();
  void createDescriptorSets();
  void establishDisplaySizeIdentity();

  /*
   * In order to enable validation layer toggle this to true and
   * follow the README.md instructions concerning the validation
   * layers. You will be required to add separate vulkan validation
   * '*.so' files in order to enable this.
   *
   * The validation layers are not shipped with the APK as they are sizeable.
   */
  bool enableValidationLayers = false;

  const std::vector<const char *> validationLayers = {
      "VK_LAYER_KHRONOS_validation"};
  const std::vector<const char *> deviceExtensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME};
  std::unique_ptr<ANativeWindow, ANativeWindowDeleter> window;
  AAssetManager *assetManager;

  VkInstance instance;
  VkDebugUtilsMessengerEXT debugMessenger;

  VkSurfaceKHR surface;

  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
  VkDevice device;

  VkSwapchainKHR swapChain;
  std::vector<VkImage> swapChainImages;
  VkFormat swapChainImageFormat;
  VkExtent2D swapChainExtent;
  VkExtent2D displaySizeIdentity;
  std::vector<VkImageView> swapChainImageViews;
  std::vector<VkFramebuffer> swapChainFrameBuffers;
  VkCommandPool commandPool;
  std::vector<VkCommandBuffer> commandBuffers;

  VkQueue graphicsQueue;
  VkQueue presentQueue;

  VkRenderPass renderPass;
  VkDescriptorSetLayout descriptorSetLayout;
  VkPipelineLayout pipelineLayout;
  VkPipeline graphicsPipeline;

  std::vector<VkBuffer> uniformBuffers;
  std::vector<VkDeviceMemory> uniformBuffersMemory;

  std::vector<VkSemaphore> imageAvailableSemaphores;
  std::vector<VkSemaphore> renderFinishedSemaphores;
  std::vector<VkFence> inFlightFences;
  VkDescriptorPool descriptorPool;
  std::vector<VkDescriptorSet> descriptorSets;

  uint32_t currentFrame = 0;
  bool orientationChanged = false;
  VkSurfaceTransformFlagBitsKHR preTransformFlag;
};

}  // namespace vkt