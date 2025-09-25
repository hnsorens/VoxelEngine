#pragma once

#include "VkZero/window.hpp"
#include <memory>
#include <vulkan/vulkan_core.h>

namespace VkZero {

struct RenderpassImpl_T
{
  virtual void record(VkCommandBuffer commandBuffer, Window* window, uint32_t currentFrame, uint32_t imageIndex) = 0;
};

}
