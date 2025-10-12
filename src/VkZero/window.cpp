#include "VkZero/window.hpp"
#include "VkZero/Internal/core_internal.hpp"
#include "VkZero/Internal/image_internal.hpp"
#include "VkZero/Internal/window_internal.hpp"
#include "VkZero/image.hpp"
#include "VkZero/info.hpp"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cstdio>
#include <limits>
#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

using namespace VkZero;

static void framebufferResizeCallback(GLFWwindow *window, int width,
                                      int height) {
  auto impl =
      reinterpret_cast<WindowImpl_T*>(glfwGetWindowUserPointer(window));
  impl->framebufferResized = true;
}

static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  }
}

Window::Window(int width, int height, const char *title)
{
  impl = new WindowImpl_T(width, height, title);
}

WindowImpl_T::WindowImpl_T(int width, int height, const char *title) :
  window([&](){
    GLFWwindow* window = glfwCreateWindow(width, height, "Voxels", nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    glfwSetKeyCallback(window, keyCallback);
    return window;
  }()),
  surface([&](){
    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(vkZero_core->instance, window, nullptr, &surface) != VK_SUCCESS) {
      throw std::runtime_error("failed to create window surface!");
    }
    return surface;
  }()),
  swapChain([&](){
    vkZero_core->chooseDevice(this);
    VkSwapchainKHR swapchain;
    createSwapChain(swapchain);
    return swapchain;
  }()),
  swapchainImages([&](){
    return createSwapchainImages();
  }()) {
}

WindowImpl_T::~WindowImpl_T()
{
    glfwDestroyWindow(window);
}

Window::~Window() {
  delete impl;

  glfwTerminate();
}

void WindowImpl_T::pollEvents() {
  deltaTime = glfwGetTime() - lastTime;
  lastTime = glfwGetTime();
  glfwPollEvents();
}

bool WindowImpl_T::shouldClose() const {
  return glfwWindowShouldClose(window);
}

GLFWwindow *WindowImpl_T::getWindow() const { return window; }

void WindowImpl_T::recreateWindow() {
  int width = 0, height = 0;
  glfwGetFramebufferSize(window, &width, &height);
  while (width == 0 || height == 0) {
    glfwGetFramebufferSize(window, &width, &height);
    glfwWaitEvents();
  }
}

bool Window::isKeyPressed(int key) {
  return glfwGetKey(impl->window, key) == GLFW_PRESS;
}

bool Window::isMouseButtonPressed(int button) {
  return glfwGetMouseButton(impl->window, button) == GLFW_PRESS;
}

void Window::hideCursor() {
  glfwSetInputMode(impl->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

double Window::getDeltaTime() const { return impl->deltaTime; }

void Window::getCursorPos(double *currentMouseX, double *currentMouseY) {
  glfwGetCursorPos(impl->window, currentMouseX, currentMouseY);
}

void Window::showCursor() {
  glfwSetInputMode(impl->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void Window::getFramebufferSize(int *width, int *height) {
  glfwGetFramebufferSize(impl->window, width, height);
}

WindowImpl_T::SwapChainSupportDetails WindowImpl_T::querySwapChainSupport(VkPhysicalDevice device) {
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

VkSurfaceFormatKHR WindowImpl_T::chooseSwapSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR> &availableFormats) {
  for (const auto &availableFormat : availableFormats) {
    if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
        availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return availableFormat;
    }
  }

  return availableFormats[0];
}

VkPresentModeKHR WindowImpl_T::chooseSwapPresentMode(
    const std::vector<VkPresentModeKHR> &availablePresentModes) {
  for (const auto &availablePresentMode : availablePresentModes) {
    if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
      return availablePresentMode;
    }
  }

  return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D WindowImpl_T::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) {
  if (capabilities.currentExtent.width !=
      std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
  } else {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

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

void WindowImpl_T::createSwapChain(VkSwapchainKHR& swapChain) {
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

  QueueFamilyIndices indices = vkZero_core->queueFamilyIndices;
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

void WindowImpl_T::recreateSwapchain() {
  cleanupSwapChain();

  createSwapChain(swapChain);

  swapchainImages = createSwapchainImages();
}

AttachmentImage WindowImpl_T::createSwapchainImages() {
  std::vector<VkImage> swapChainImages;
    // vkGetSwapchainImagesKHR(device, swapChain, &swapchainImageCount, nullptr);
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

    return AttachmentImage(new MultiImageImpl_T(swapChainImages.data(), swapChainImageViews.data(), swapChainImages.size()));
}

void WindowImpl_T::cleanupSwapChain() {
  vkDeviceWaitIdle(device);

  vkDestroySwapchainKHR(device, swapChain, nullptr);
}

uint32_t Window::getWidth()
{
  return impl->swapChainExtent.width;
}

uint32_t Window::getHeight()
{
  return impl->swapChainExtent.height;
}

VkSwapchainKHR WindowImpl_T::getSwapChain()
{
  return swapChain;
}

AttachmentImage& Window::getSwapChainImages()
{
  return impl->swapchainImages;
}

void Window::pollEvents()
{
  impl->pollEvents();
}

bool Window::shouldClose()
{
  return impl->shouldClose();
}