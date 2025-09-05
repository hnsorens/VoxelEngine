#pragma once

#include "VkZero/window.hpp"
#include "VkZero/image.hpp"
#include <GLFW/glfw3.h>

namespace VkZero
{
    struct WindowImpl_T
    {
        uint32_t swapchainImageCount;
        GLFWwindow *window;
        VkSurfaceKHR surface;
        VkSwapchainKHR swapChain;
        // std::vector<VkImage> swapChainImages;
        // std::vector<VkImageView> swapChainImageViews;
        AttachmentImage swapchainImages;
        VkFormat swapChainImageFormat;
        VkExtent2D swapChainExtent;
        double deltaTime = 0;
        double lastTime = 0;

        
        VkPhysicalDevice physicalDevice;
        VkDevice device;

        bool framebufferResized = false;

        struct SwapChainSupportDetails {
            VkSurfaceCapabilitiesKHR capabilities;
            std::vector<VkSurfaceFormatKHR> formats;
            std::vector<VkPresentModeKHR> presentModes;
        };

        void pollEvents();
        bool shouldClose() const;
        void recreateWindow();
        GLFWwindow *getWindow() const;
        VkSwapchainKHR getSwapChain();
        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);
        void createSwapChain(std::unique_ptr<VulkanContext>& ctx, VkSwapchainKHR& swapchain);
        void recreateSwapchain(std::unique_ptr<VulkanContext>& ctx);
        void cleanupSwapChain();
        AttachmentImage createSwapchainImages();

        WindowImpl_T(std::unique_ptr<VulkanContext>& vulkanContext, int width, int height, const char *title);
        ~WindowImpl_T();
    };
}