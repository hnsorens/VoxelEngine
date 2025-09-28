#pragma once

#include "VkZero/render_pass.hpp"
#include "VkZero/Internal/renderpass_internal.hpp"
#include <memory>

namespace VkZero {
struct RaytracingRenderpassImpl_T : public RenderpassImpl_T{
  RaytracingRenderpassImpl_T(
      std::vector<
          std::pair<RaytracingPipelineImpl_T *, PushConstantDataImpl_T *>>
          pipelines, std::function<void(VkCommandBuffer, uint32_t)> before, std::function<void(VkCommandBuffer, uint32_t)> after);

  void record(VkCommandBuffer commandBuffer, WindowImpl_T* window, uint32_t currentFrame,
              uint32_t imageIndex) override;

  void recreateSwapchain(WindowImpl_T* window) override {}

  PFN_vkCmdTraceRaysKHR vkCmdTraceRaysKHR;
  std::vector<std::pair<RaytracingPipelineImpl_T *, PushConstantDataImpl_T *>>
      pipelines;
  std::function<void(VkCommandBuffer, uint32_t)> before;
  std::function<void(VkCommandBuffer, uint32_t)> after;
};
} // namespace VkZero
