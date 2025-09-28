#pragma once

#include "VkZero/Internal/core_internal.hpp"
#include "VkZero/image.hpp"
#include "VkZero/window.hpp"
#include <GLFW/glfw3.h>
#include <chrono>
#include <iostream>
#include <thread>
#include <vulkan/vulkan_core.h>

namespace VkZero {
struct WindowImpl_T {
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
  VkSurfaceFormatKHR chooseSwapSurfaceFormat(
      const std::vector<VkSurfaceFormatKHR> &availableFormats);
  VkPresentModeKHR chooseSwapPresentMode(
      const std::vector<VkPresentModeKHR> &availablePresentModes);
  VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);
  void createSwapChain(VkSwapchainKHR &swapchain);
  void recreateSwapchain();
  void cleanupSwapChain();
  AttachmentImage createSwapchainImages();

  WindowImpl_T(int width, int height, const char *title);
  ~WindowImpl_T();

  bool nextImage(uint32_t &imageIndex, VkSemaphore &imageSemaphore) {
    VkResult result = vkAcquireNextImageKHR(
        VkZero::vkZero_core->device, getSwapChain(), UINT64_MAX, imageSemaphore,
        VK_NULL_HANDLE, &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
      recreateSwapchain();
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
      throw std::runtime_error("failed to acquire swap chain image!");
    }
    return result == VK_ERROR_OUT_OF_DATE_KHR;
  }

  bool present(uint32_t &imageIndex, VkSemaphore &signalSemaphore) {
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &signalSemaphore;

    VkSwapchainKHR swapChains[] = {getSwapChain()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &imageIndex;
    VkResult result =
        vkQueuePresentKHR(VkZero::vkZero_core->presentQueue, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
        framebufferResized) {
      framebufferResized = false;
      std::cout << "Recreating swapchain..." << std::endl;
      recreateSwapchain();
      // Add a small delay to prevent excessive recreation
      std::this_thread::sleep_for(std::chrono::milliseconds(16));
    } else if (result != VK_SUCCESS) {
      throw std::runtime_error("failed to present swap chain image!");
    }

    return result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR;
  }
};
}
