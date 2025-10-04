#pragma once

#include <memory>
#include <mutex>
#include <optional>
#include <vector>
#include <vulkan/vulkan_core.h>

#include "VkZero/info.hpp"

namespace VkZero
{
  struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
      return graphicsFamily.has_value() && presentFamily.has_value();
    }
  };
  struct VkZeroCoreImpl_T
  {
    static VkZeroCoreImpl_T* getInstance()
    {
      static VkZeroCoreImpl_T instance;
      return &instance;
    }

    VkZeroCoreImpl_T();
    ~VkZeroCoreImpl_T();

    bool checkValidationLayerSupport();
    std::vector<const char *> getRequiredExtensions();
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
    bool isDeviceSuitable(VkPhysicalDevice device, struct WindowImpl_T* window);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    struct SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    struct QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, struct WindowImpl_T* window);

    /* Debug Vulkan Functions */
    VkResult CreateDebugUtilsMessengerEXT(
        VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
        const VkAllocationCallbacks *pAllocator,
        VkDebugUtilsMessengerEXT *pDebugMessenger);
    void DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                      VkDebugUtilsMessengerEXT debugMessenger,
                                      const VkAllocationCallbacks *pAllocator);

    void createInstance();
    void setupDebugMessenger();
    void pickPhysicalDevice(struct WindowImpl_T* window);
    void createLogicalDevice(struct WindowImpl_T* window);
    void chooseDevice(struct WindowImpl_T* window);

    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    QueueFamilyIndices queueFamilyIndices;
  };
}