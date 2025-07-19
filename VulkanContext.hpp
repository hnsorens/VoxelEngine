#pragma once
#include <vulkan/vulkan.h>
#include <vector>

class VulkanContext {
public:
    VulkanContext();
    ~VulkanContext();

    void init(GLFWwindow* window);
    void cleanup();
    void recreateSwapChain();

    VkDevice getDevice() const;
    VkPhysicalDevice getPhysicalDevice() const;
    VkInstance getInstance() const;
    VkQueue getGraphicsQueue() const;
    VkQueue getPresentQueue() const;
    VkSwapchainKHR getSwapChain() const;
    VkExtent2D getSwapChainExtent() const;
    VkFormat getSwapChainImageFormat() const;
    const std::vector<VkImage>& getSwapChainImages() const;
    const std::vector<VkImageView>& getSwapChainImageViews() const;
    const std::vector<VkFramebuffer>& getSwapChainFramebuffers() const;

private:
    void createInstance();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createSwapChain();
    void createImageViews();
    void createRenderPass();
    void createFramebuffers();
    void cleanupSwapChain();

    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkSurfaceKHR surface;
    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;
}; 