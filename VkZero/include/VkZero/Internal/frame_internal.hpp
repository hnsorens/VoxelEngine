#pragma once

#include "VkZero/frame.hpp"
#include <vulkan/vulkan_core.h>

namespace VkZero
{
struct FrameImpl_T {
  FrameImpl_T(std::vector<RenderpassImpl_T *> renderpasses, WindowImpl_T* window);
  void draw();

  std::vector<VkSemaphore> imageAvailableSemaphores;
  std::vector<VkSemaphore> renderFinishedSemaphores;
  std::vector<VkFence> inFlightFences;
  int currentFrame = 0;
  std::vector<RenderpassImpl_T *> renderpasses;
  std::vector<VkCommandBuffer> commandBuffers[MAX_FRAMES_IN_FLIGHT];
  WindowImpl_T *window;
  VkCommandPool commandPool;
};
}