#pragma once

#include "image.hpp"
#include <GLFW/glfw3.h>
#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

class WindowManager {
public:
  WindowManager(std::unique_ptr<class VulkanContext>& vulkanContext, int width, int height, const char *title);
  ~WindowManager();

  struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
  };

  void pollEvents();
  bool shouldClose() const;
  void recreateWindow();
  GLFWwindow *getWindow() const;
  bool isKeyPressed(int key);
  bool isMouseButtonPressed(int button);
  void hideCursor();
  double getDeltaTime() const;
  void getCursorPos(double *currentMouseX, double *currentMouseY);
  void showCursor();
  void getFramebufferSize(int *width, int *height);

  VkExtent2D getSwapChainExtent();
  VkSwapchainKHR getSwapChain();

  bool framebufferResized = false;

  SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
  VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
  VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
  VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);
  void createSwapChain(std::unique_ptr<VulkanContext>& ctx, VkSwapchainKHR& swapchain);
  void recreateSwapchain(std::unique_ptr<VulkanContext>& ctx);
  void cleanupSwapChain();
  AttachmentImage createSwapchainImages();
  AttachmentImage& getSwapChainImages();
  uint32_t swapchainImageCount;

private:
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

  friend class VulkanContext;
};