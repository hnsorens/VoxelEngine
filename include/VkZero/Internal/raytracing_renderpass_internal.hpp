#pragma once

#include "VkZero/render_pass.hpp"
#include "VkZero/Internal/renderpass_internal.hpp"
#include <memory>

namespace VkZero {
struct RaytracingRenderpassImpl_T : public RenderpassImpl_T{
  RaytracingRenderpassImpl_T(
      std::vector<
          std::pair<RaytracingPipelineImpl_T *, PushConstantDataImpl_T *>>
          pipelines);

  void record(VkCommandBuffer commandBuffer, Window* window, uint32_t currentFrame,
              uint32_t imageIndex) override;

  PFN_vkCmdTraceRaysKHR vkCmdTraceRaysKHR;
  std::vector<std::pair<RaytracingPipelineImpl_T *, PushConstantDataImpl_T *>>
      pipelines;
};
} // namespace VkZero
