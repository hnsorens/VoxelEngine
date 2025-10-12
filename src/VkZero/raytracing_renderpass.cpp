#include "VkZero/Internal/raytracing_renderpass_internal.hpp"
#include "VkZero/Internal/graphics_renderpass_internal.hpp"
#include "VkZero/Internal/raytracing_pipeline_internal.hpp"

using namespace VkZero;

RaytracingRenderpassImpl_T::RaytracingRenderpassImpl_T(
    std::vector<std::pair<RaytracingPipelineImpl_T *, PushConstantDataImpl_T *>>
        pipelines, std::function<void(VkCommandBuffer, uint32_t)> before, std::function<void(VkCommandBuffer, uint32_t)> after)
    : pipelines{pipelines}, before(before), after(after) {
  vkCmdTraceRaysKHR = reinterpret_cast<PFN_vkCmdTraceRaysKHR>(
      vkGetDeviceProcAddr(vkZero_core->device, "vkCmdTraceRaysKHR"));
}

void RaytracingRenderpassImpl_T::record(VkCommandBuffer commandBuffer,
                                        WindowImpl_T* window,
                                        uint32_t currentFrame,
                                        uint32_t imageIndex) {
  for (auto &[pipeline, pushConstant] : pipelines) {
    before(commandBuffer, currentFrame);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
                      pipeline->pipeline);
    pipeline->bindResources(commandBuffer, currentFrame);
    *(uint32_t *)(pushConstant->data) = 0;
    vkCmdPushConstants(commandBuffer, pipeline->pipelineLayout,
                       VK_SHADER_STAGE_RAYGEN_BIT_KHR, 0, 8,
                       pushConstant->data);
    vkCmdTraceRaysKHR(commandBuffer, &pipeline->raygenRegion,
                      &pipeline->missRegion, &pipeline->hitRegion,
                      &pipeline->callableRegion, pipeline->width,
                      pipeline->height, 1);

    VkMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT |
                            VK_ACCESS_SHADER_READ_BIT; // Ensure writes from the
                                                       // first trace finish
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT |
                            VK_ACCESS_SHADER_WRITE_BIT; // Ensure the second
                                                        // trace can read them
    barrier.pNext = 0;
    vkCmdPipelineBarrier(
        commandBuffer,
        VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR, // Source: First
                                                      // trace rays
                                                      // execution
        VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR, // Destination:
                                                      // Second trace rays
                                                      // execution
        0, 1, &barrier, 0, nullptr, 0, nullptr);
    *(uint32_t *)(pushConstant->data) = 1;
    vkCmdPushConstants(commandBuffer, pipeline->pipelineLayout,
                       VK_SHADER_STAGE_RAYGEN_BIT_KHR, 0, 4,
                       pushConstant->data);
    vkCmdTraceRaysKHR(commandBuffer, &pipeline->raygenRegion,
                      &pipeline->missRegion, &pipeline->hitRegion,
                      &pipeline->callableRegion, pipeline->width,
                      pipeline->height, 1);
    vkCmdPipelineBarrier(
        commandBuffer,
        VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR, // Source: First
                                                      // trace rays
                                                      // execution
        VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR, // Destination:
                                                      // Second trace rays
                                                      // execution
        0, 1, &barrier, 0, nullptr, 0, nullptr);
    *(uint32_t *)(pushConstant->data) = 2;
    vkCmdPushConstants(commandBuffer, pipeline->pipelineLayout,
                       VK_SHADER_STAGE_RAYGEN_BIT_KHR, 0, 4,
                       pushConstant->data);
    vkCmdTraceRaysKHR(commandBuffer, &pipeline->raygenRegion,
                      &pipeline->missRegion, &pipeline->hitRegion,
                      &pipeline->callableRegion, pipeline->width,
                      pipeline->height, 1);
    after(commandBuffer, currentFrame);
  }
}
RaytracingRenderpassBase::RaytracingRenderpassBase(
      std::vector<
          std::pair<RaytracingPipelineImpl_T *, PushConstantDataImpl_T *>>
          pipelines, std::function<void(void*, uint32_t)> before, std::function<void(void*, uint32_t)> after) {
    impl = new RaytracingRenderpassImpl_T(pipelines, before, after);
  }
