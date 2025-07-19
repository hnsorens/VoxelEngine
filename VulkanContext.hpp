#pragma once


#include "WindowManager.hpp"
#include <memory>
#include <optional>
#include <vector>
#include <vulkan/vulkan_core.h>

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


class VulkanContext {
public:
    VulkanContext();
    ~VulkanContext();

    void init(std::unique_ptr<WindowManager>& window);
    void cleanup();
    void recreateSwapchain(std::unique_ptr<WindowManager>& windowManager);

    VkDevice getDevice() const;
    VkSurfaceKHR getSurface() const;
    VkPhysicalDevice getPhysicalDevice() const;
    VkInstance getInstance() const;
    VkQueue getGraphicsQueue() const;
    VkQueue getPresentQueue() const;
    VkSwapchainKHR getSwapChain() const;
    VkExtent2D getSwapChainExtent() const;
    VkFormat getSwapChainImageFormat() const;
    VkRenderPass getRenderPass() const;
    const std::vector<VkImage>& getSwapChainImages() const;
    const std::vector<VkImageView>& getSwapChainImageViews() const;
    const std::vector<VkFramebuffer>& getSwapChainFramebuffers() const;


private:

    bool checkValidationLayerSupport();
    std::vector<const char*> getRequiredExtensions();
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    bool isDeviceSuitable(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    struct SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    struct QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, std::unique_ptr<WindowManager>& windowManager);

    /* Debug Vulkan Functions */
    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

    void createInstance();
    void setupDebugMessenger();
    void createSurface(GLFWwindow* window);
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createSwapChain(std::unique_ptr<WindowManager>& windowManager);
    void createImageViews();
    void createRenderPass();
    void createFramebuffers();
    void cleanupSwapChain();

    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkSurfaceKHR surface;
    VkSwapchainKHR swapChain;
    VkRenderPass renderPass;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;
}; 