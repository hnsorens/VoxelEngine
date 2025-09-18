#pragma once

#include "VkZero/graphics_pipeline.hpp"

namespace VkZero
{
struct GraphicsPipelineImpl_T {
  GraphicsPipelineImpl_T(ShaderGroupImpl *shaderGroup,
                         std::vector<ShaderResourceSetImpl_T *> resources);

  void create_pipeline(VkDevice device, VkRenderPass renderPass);
  
  void bindResources(VkCommandBuffer commandBuffer, int currentFrame);

  std::vector<ShaderResourceSetImpl_T *> resources;
  ShaderGroupImpl *m_shaderGroup;
  VkGraphicsPipelineCreateInfo pipelineInfo{};
  VkPipelineLayout pipelineLayout;
  VkPipeline pipeline = VK_NULL_HANDLE;
};
}
