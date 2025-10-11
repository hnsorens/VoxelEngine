// #pragma once

// #include "VkZero/image.hpp"
// #include <GLFW/glfw3.h>

// namespace VkZero
// {
//     struct ImageData_T
//     {
//         VkImage image;
//         VkImageView view;
//         VkDeviceMemory memory;
//     };

//     struct StagingData_T
//     {
//         VkBuffer buffer;
//         VkDeviceMemory memory;
//     };

//     struct ImageImpl_T
//     {
//         uint32_t width, height, depth;
//         VkSampler sampler;
//         VkImageLayout imageLayout;
//         VkFormat format;
//     };

//         struct SwapChainSupportDetails {
//       VkSurfaceCapabilitiesKHR capabilities;
//       std::vector<VkSurfaceFormatKHR> formats;
//       std::vector<VkPresentModeKHR> presentModes;
//     };

//     void pollEvents();
//     bool shouldClose();
//     void recreateWindow();
//     GLFWwindow *getWindow();
//     VkSwapchainKHR getSwapChain();
//     SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
//     VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
//     VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
//     VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);
//     void createSwapChain(VkSwapchainKHR& swapchain);
//     void recreateSwapchain();
//     void cleanupSwapChain();
//     AttachmentImage createSwapchainImages();
// }