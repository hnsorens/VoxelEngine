#include "VkZero/raytracing_pipeline.hpp"

namespace VkZero {

struct RaytracingPipelineImpl_T {
RaytracingPipelineImpl_T(ShaderGroupImpl *shaderGroup,
                          std::vector<ShaderResourceSetImpl_T *> resources);

  void bindResources(VkCommandBuffer commandBuffer, int currentFrame);

  ShaderGroupImpl *m_shaderGroup;
  VkRayTracingPipelineCreateInfoKHR pipelineInfo{};
  VkPipelineLayout pipelineLayout;
  VkPipeline pipeline = VK_NULL_HANDLE;

  VkStridedDeviceAddressRegionKHR raygenRegion;
  VkStridedDeviceAddressRegionKHR missRegion;
  VkStridedDeviceAddressRegionKHR hitRegion;
  VkStridedDeviceAddressRegionKHR callableRegion;
  VkDeviceSize sbtSize;
  VkBuffer sbtBuffer;
  VkDeviceMemory sbtMemory;

  std::vector<ShaderResourceSetImpl_T *> resources;
};
}
