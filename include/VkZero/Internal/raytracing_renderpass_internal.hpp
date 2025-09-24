#pragma once

#include "VkZero/render_pass.hpp"

namespace VkZero {
struct RaytracingRenderpassImpl_T {
  RaytracingRenderpassImpl_T(
      std::vector<
          std::pair<RaytracingPipelineImpl_T *, PushConstantDataImpl_T *>>
          pipelines);

  void record(VkCommandBuffer commandBuffer, uint32_t currentFrame,
              uint32_t imageIndex);

  PFN_vkCmdTraceRaysKHR vkCmdTraceRaysKHR;
  std::vector<std::pair<RaytracingPipelineImpl_T *, PushConstantDataImpl_T *>>
      pipelines;
};
} // namespace VkZero
