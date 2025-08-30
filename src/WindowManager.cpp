#include "WindowManager.hpp"
#include "VulkanContext.hpp"
#include "image.hpp"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <limits>
#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

static void framebufferResizeCallback(GLFWwindow *window, int width,
                                      int height) {
  auto windowManager =
      reinterpret_cast<WindowManager *>(glfwGetWindowUserPointer(window));
  windowManager->framebufferResized = true;
}

static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  }
}

WindowManager::WindowManager(std::unique_ptr<VulkanContext>& vulkanContext, int width, int height, const char *title) : 
  window([&](){
    GLFWwindow* window = glfwCreateWindow(width, height, "Voxels", nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    glfwSetKeyCallback(window, keyCallback);
    return window;
  }()),
  surface([&](){
    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(vulkanContext->getInstance(), window, nullptr, &surface) != VK_SUCCESS) {
      throw std::runtime_error("failed to create window surface!");
    }
    return surface;
  }()),
  swapChain([&](){
    vulkanContext->chooseDevice(this);
    VkSwapchainKHR swapchain;
    createSwapChain(vulkanContext, swapchain);
    return swapchain;
  }()),
  swapchainImages([&](){
    return createSwapchainImages();
  }()) {
}

WindowManager::~WindowManager() {
  glfwDestroyWindow(window);

  glfwTerminate();
}

void WindowManager::pollEvents() {
  deltaTime = glfwGetTime() - lastTime;
  lastTime = glfwGetTime();
  glfwPollEvents();
}

bool WindowManager::shouldClose() const {
  return glfwWindowShouldClose(window);
}

GLFWwindow *WindowManager::getWindow() const { return window; }

void WindowManager::recreateWindow() {
  int width = 0, height = 0;
  glfwGetFramebufferSize(window, &width, &height);
  while (width == 0 || height == 0) {
    glfwGetFramebufferSize(window, &width, &height);
    glfwWaitEvents();
  }
}

bool WindowManager::isKeyPressed(int key) {
  return glfwGetKey(window, key) == GLFW_PRESS;
}

bool WindowManager::isMouseButtonPressed(int button) {
  return glfwGetMouseButton(window, button) == GLFW_PRESS;
}

void WindowManager::hideCursor() {
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

double WindowManager::getDeltaTime() const { return deltaTime; }

void WindowManager::getCursorPos(double *currentMouseX, double *currentMouseY) {
  glfwGetCursorPos(window, currentMouseX, currentMouseY);
}

void WindowManager::showCursor() {
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void WindowManager::getFramebufferSize(int *width, int *height) {
  glfwGetFramebufferSize(window, width, height);
}

WindowManager::SwapChainSupportDetails WindowManager::querySwapChainSupport(VkPhysicalDevice device) {
  SwapChainSupportDetails details;

  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface,
                                            &details.capabilities);

  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

  if (formatCount != 0) {
    details.formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount,
                                         details.formats.data());
  }

  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount,
                                            nullptr);

  if (presentModeCount != 0) {
    details.presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        device, surface, &presentModeCount, details.presentModes.data());
  }

  return details;
}

VkSurfaceFormatKHR WindowManager::chooseSwapSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR> &availableFormats) {
  for (const auto &availableFormat : availableFormats) {
    if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
        availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return availableFormat;
    }
  }

  return availableFormats[0];
}

VkPresentModeKHR WindowManager::chooseSwapPresentMode(
    const std::vector<VkPresentModeKHR> &availablePresentModes) {
  for (const auto &availablePresentMode : availablePresentModes) {
    if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
      return availablePresentMode;
    }
  }

  return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D WindowManager::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) {
  if (capabilities.currentExtent.width !=
      std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
  } else {
    int width, height;
    getFramebufferSize(&width, &height);

    VkExtent2D actualExtent = {static_cast<uint32_t>(width),
                               static_cast<uint32_t>(height)};

    actualExtent.width =
        std::clamp(actualExtent.width, capabilities.minImageExtent.width,
                   capabilities.maxImageExtent.width);
    actualExtent.height =
        std::clamp(actualExtent.height, capabilities.minImageExtent.height,
                   capabilities.maxImageExtent.height);

    return actualExtent;
  }
}

void WindowManager::createSwapChain(std::unique_ptr<VulkanContext>& ctx, VkSwapchainKHR& swapChain) {
  SwapChainSupportDetails swapChainSupport =
      querySwapChainSupport(physicalDevice);

  VkSurfaceFormatKHR surfaceFormat =
      chooseSwapSurfaceFormat(swapChainSupport.formats);
  VkPresentModeKHR presentMode =
      chooseSwapPresentMode(swapChainSupport.presentModes);
  VkExtent2D extent =
      chooseSwapExtent(swapChainSupport.capabilities);

  uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
  if (swapChainSupport.capabilities.maxImageCount > 0 &&
      imageCount > swapChainSupport.capabilities.maxImageCount) {
    imageCount = swapChainSupport.capabilities.maxImageCount;
  }

  swapchainImageCount = std::min(static_cast<int>(imageCount), MAX_FRAMEBUFFER_COUNT);

  VkSwapchainCreateInfoKHR createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface = surface;

  createInfo.minImageCount = swapchainImageCount;
  createInfo.imageFormat = surfaceFormat.format;
  createInfo.imageColorSpace = surfaceFormat.colorSpace;
  createInfo.imageExtent = extent;
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  QueueFamilyIndices indices = ctx->queueFamilyIndices;
  uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(),
                                   indices.presentFamily.value()};

  if (indices.graphicsFamily != indices.presentFamily) {
    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = queueFamilyIndices;
  } else {
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  }

  createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  createInfo.presentMode = presentMode;
  createInfo.clipped = VK_TRUE;

  if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create swap chain!");
  }

  swapChainImageFormat = surfaceFormat.format;
  swapChainExtent = extent;
}

void WindowManager::recreateSwapchain(std::unique_ptr<VulkanContext>& ctx) {
  cleanupSwapChain();

  createSwapChain(ctx, swapChain);

  swapchainImages = createSwapchainImages();
}

AttachmentImage WindowManager::createSwapchainImages() {
  std::vector<VkImage> swapChainImages;
    vkGetSwapchainImagesKHR(device, swapChain, &swapchainImageCount, nullptr);
    swapChainImages.resize(swapchainImageCount);
    vkGetSwapchainImagesKHR(device, swapChain, &swapchainImageCount,
                            swapChainImages.data());

    std::vector<VkImageView> swapChainImageViews;
    swapChainImageViews.resize(swapChainImages.size());

    for (size_t i = 0; i < swapChainImages.size(); i++) {
      VkImageViewCreateInfo createInfo{};
      createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
      createInfo.image = swapChainImages[i];
      createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
      createInfo.format = swapChainImageFormat;
      createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      createInfo.subresourceRange.baseMipLevel = 0;
      createInfo.subresourceRange.levelCount = 1;
      createInfo.subresourceRange.baseArrayLayer = 0;
      createInfo.subresourceRange.layerCount = 1;

      if (vkCreateImageView(device, &createInfo, nullptr,
                            &swapChainImageViews[i]) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image views!");
      }
    }
    return AttachmentImage(swapChainImages, swapChainImageViews);
}

void WindowManager::cleanupSwapChain() {
  vkDeviceWaitIdle(device);

  vkDestroySwapchainKHR(device, swapChain, nullptr);
}

VkExtent2D WindowManager::getSwapChainExtent()
{
  return swapChainExtent;
}

VkSwapchainKHR WindowManager::getSwapChain()
{
  return swapChain;
}

AttachmentImage& WindowManager::getSwapChainImages()
{
  return swapchainImages;
}