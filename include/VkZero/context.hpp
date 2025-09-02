#pragma once

#include "VkZero/window.hpp"
#include <memory>
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

  class VulkanContext {
  public:
    VulkanContext();
    ~VulkanContext();

    void cleanup();

    VkDevice getDevice() const;
    VkPhysicalDevice getPhysicalDevice() const;
    VkInstance getInstance() const;
    VkQueue getGraphicsQueue() const;
    VkQueue getPresentQueue() const;
    const QueueFamilyIndices &getQueueFamilyIndices() const;

  private:
    bool checkValidationLayerSupport();
    std::vector<const char *> getRequiredExtensions();
    void populateDebugMessengerCreateInfo(
        VkDebugUtilsMessengerCreateInfoEXT &createInfo);
    bool isDeviceSuitable(VkPhysicalDevice device, class WindowManager* window);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    struct SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    struct QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, class WindowManager* window);

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
    void pickPhysicalDevice(class WindowManager* window);
    void createLogicalDevice(class WindowManager* window);

    void chooseDevice(WindowManager* window);

    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    QueueFamilyIndices queueFamilyIndices;

    friend class WindowManager;
  };
}