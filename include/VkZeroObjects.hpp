
#pragma once
#include "Camera.hpp"
#include "VkZero/frame.hpp"
#include "VkZero/raytracing_pipeline.hpp"
#include "VkZero/shader_resource_set.hpp"
#include "VkZero/uniform_buffer.hpp"
#include "VoxelWorld.hpp"
#include "VkZero/image.hpp"
#include "shaders.hpp"
#include <memory>
#include <vector>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define RAYTRACE_WIDTH 1920
#define RAYTRACE_HEIGHT 1080
namespace VkZero{
class Window;
}
class VkZeroObjects {
public:
  VkZeroObjects(
            std::unique_ptr<VoxelWorld> &voxelWorld,
            std::unique_ptr<class Camera> &camera, std::unique_ptr<VkZero::Window> &window, std::function<void(VkCommandBuffer, uint32_t)> after);
  ~VkZeroObjects();
  

  using RaytracingPushConstants = VkZero::ShaderPushConstants<VkZero::PushConstant<RaytracingPushConstant, VkZero::SHADER_RGEN>>;
  using RaytracingShaderGroup = VkZero::ShaderGroup<RaytracingPushConstants, main_rmiss, main_rgen>;
  using RaytracingResourceSet = VkZero::ShaderResourceSet<
    VkZero::ResourceBinding<VkZero::SwapImage, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VkZero::SHADER_RGEN, 0, 1>,
    VkZero::ResourceBinding<VkZero::UniformBuffer<TransformUBO>, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VkZero::SHADER_RGEN, 1, 1>,
    VkZero::ResourceBinding<VkZero::SwapImage, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VkZero::SHADER_RGEN, 2, 1>,
    VkZero::ResourceBinding<VkZero::StagedSharedImage, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VkZero::SHADER_RGEN, 3, 512>,
    VkZero::ResourceBinding<VkZero::StagedSharedImage, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VkZero::SHADER_RGEN, 4, 1>,
    VkZero::ResourceBinding<VkZero::SwapImage, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VkZero::SHADER_RGEN, 5, 1>,
    VkZero::ResourceBinding<VkZero::SwapImage, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VkZero::SHADER_RGEN, 6, 1>,
    VkZero::ResourceBinding<VkZero::SwapImage, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VkZero::SHADER_RGEN, 7, 1>,
    VkZero::ResourceBinding<VkZero::SwapImage, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VkZero::SHADER_RGEN, 8, 1>
  >;
  using RaytracingPipeline = VkZero::RaytracingPipeline<RaytracingShaderGroup, RaytracingResourceSet>;
  using RaytracingPushConstantData = VkZero::PushConstantData<RaytracingPushConstant>;
  using RaytracingRenderPass_t = VkZero::RaytracingRenderPass<VkZero::RaytracingRenderPassPipeline<RaytracingPushConstantData, RaytracingPipeline>>;

  VkZero::SwapImage raytracingStorageImage;
  VkZero::SwapImage raytracingPositionStorageImage;
  VkZero::SwapImage raytracingLightStorageImageX;
  VkZero::SwapImage raytracingLightStorageImageY;
  VkZero::SwapImage raytracingLightStorageImageZ;
  VkZero::SwapImage raytracingLightStorageImageW;

  RaytracingPushConstants raytracingPushConstants;
  RaytracingShaderGroup raytracingShaderGroup;
  RaytracingResourceSet raytracingResourceSet;
  RaytracingPipeline raytracingPipeline;  
  RaytracingPushConstantData raytracingPushConstantData;
  RaytracingRenderPass_t raytracingRenderPass;

  using PipelineShaderPushConstant = VkZero::ShaderPushConstants<>;
  using PipelineShaderGroup = VkZero::ShaderGroup<PipelineShaderPushConstant, main_vert, main_frag>;
  using PipelineShaderResources = VkZero::ShaderResourceSet<VkZero::ResourceBinding<VkZero::SwapImage, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VkZero::SHADER_FRAGMENT, 0, 1>>;
  using Pipeline = VkZero::GraphicsPipeline<PipelineShaderGroup, PipelineShaderResources>;
  using PipelineRenderPassResources = VkZero::RenderPassResourceSet<VkZero::RenderPassResource<"output">>;
  using PipelineRenderPass = VkZero::GraphicsRenderpass<PipelineRenderPassResources, Pipeline>;

  PipelineShaderPushConstant graphicsPushConstants;
  PipelineShaderGroup graphicsShaderGroup;
  PipelineShaderResources graphicsResourceSet;
  Pipeline graphicsPipeline;
  PipelineRenderPassResources graphicsRenderpassAttachmentSet;
  PipelineRenderPass graphicsRenderPass;

  VkZero::Frame<RaytracingRenderPass_t, PipelineRenderPass> frame;

  void draw();
};
