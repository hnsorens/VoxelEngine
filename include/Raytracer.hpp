#pragma once
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

class Raytracer {
public:
  Raytracer(std::unique_ptr<class CommandManager> &commandManager,
            std::unique_ptr<VoxelWorld> &voxelWorld,
            std::unique_ptr<class Camera> &camera, std::function<void(VkCommandBuffer, uint32_t)> after);
  ~Raytracer();

  void createRaytracingPipeline(
      VkDevice device, std::vector<VkBuffer> &uniformBuffer,
      VkZero::StagedSharedImage* voxelImage,
      VkZero::StagedSharedImage* voxelChunkMapImage);
  void createRaytracingResources(
      std::unique_ptr<class CommandManager> &commandManager);
  void recordRaytracingCommandBuffer(VkCommandBuffer commandBuffer,
                                     uint32_t imageIndex, uint8_t section);

  const VkPipeline &getPipeline() const;
  const VkPipelineLayout &getPipelineLayout() const;
  const VkDescriptorSet &getDescriptorSet(int i) const;
  VkZero::SwapImage* getStorageImage();

// private:

  using RaytracingPushConstants = VkZero::ShaderPushConstants<VkZero::PushConstant<RaytracingPushConstant, VkZero::SHADER_RGEN>>;
  using RaytracingShaderGroup = VkZero::ShaderGroup<RaytracingPushConstants, main_rmiss, main_rgen>;
  using RaytracingResourceSet = VkZero::ShaderResourceSet<
    VkZero::ResourceBinding<VkZero::SwapImage, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VkZero::SHADER_RGEN, 0, 1>,
    VkZero::ResourceBinding<VkZero::SwapImage, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VkZero::SHADER_RGEN, 1, 1>,
    VkZero::ResourceBinding<VkZero::SwapImage, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VkZero::SHADER_RGEN, 2, 1>,
    VkZero::ResourceBinding<VkZero::StagedSharedImage, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VkZero::SHADER_RGEN, 3, 512>,
    VkZero::ResourceBinding<VkZero::StagedSharedImage, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VkZero::SHADER_RGEN, 4, 1>,
    VkZero::ResourceBinding<VkZero::SwapImage, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VkZero::SHADER_RGEN, 5, 1>,
    VkZero::ResourceBinding<VkZero::SwapImage, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VkZero::SHADER_RGEN, 6, 1>,
    VkZero::ResourceBinding<VkZero::SwapImage, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VkZero::SHADER_RGEN, 7, 1>,
    VkZero::ResourceBinding<VkZero::SwapImage, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VkZero::SHADER_RGEN, 8, 1>
  >;
  using Pipeline = VkZero::RaytracingPipeline<RaytracingShaderGroup, RaytracingResourceSet>;
  using RaytracingPushConstantData = VkZero::PushConstantData<RaytracingPushConstant>;
  using RaytracingRenderPass_t = VkZero::RaytracingRenderPass<VkZero::RaytracingRenderPassPipeline<RaytracingPushConstantData, Pipeline>>;

  VkZero::SwapImage raytracingStorageImage;
  VkZero::SwapImage raytracingPositionStorageImage;
  VkZero::SwapImage raytracingLightStorageImageX;
  VkZero::SwapImage raytracingLightStorageImageY;
  VkZero::SwapImage raytracingLightStorageImageZ;
  VkZero::SwapImage raytracingLightStorageImageW;

  RaytracingPushConstants raytracingPushConstants;
  RaytracingShaderGroup group;
  RaytracingResourceSet set1;
  Pipeline something;  
  RaytracingPushConstantData data;
  RaytracingRenderPass_t renderPass;

  VkPipeline raytracingPipeline;
  VkPipelineLayout raytracingPipelineLayout;
  VkDescriptorSetLayout raytracingDescriptorSetLayout;
  std::vector<VkDescriptorSet> raytracingDescriptorSets;
};
