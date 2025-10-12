#pragma once

#include "VkZero/window.hpp"
#include <memory>
#include <vulkan/vulkan_core.h>

namespace VkZero {

struct RenderpassImpl_T
{
  virtual void record(VkCommandBuffer commandBuffer, WindowImpl_T* window, uint32_t currentFrame, uint32_t imageIndex) = 0;
  virtual void recreateSwapchain(WindowImpl_T* window) = 0;
};

}
