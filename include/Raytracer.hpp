#pragma once
#include "VoxelWorld.hpp"
#include "image.hpp"
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
            std::unique_ptr<class VulkanContext> &vulkanContext,
            std::unique_ptr<VoxelWorld> &voxelWorld,
            std::unique_ptr<class Camera> &camera);
  ~Raytracer();

  void createRaytracingPipeline(
      VkDevice device, std::vector<VkBuffer> &uniformBuffer,
      StagedSharedImage* voxelImage,
      StagedSharedImage* voxelChunkMapImage);
  void createRaytracingResources(
      std::unique_ptr<class CommandManager> &commandManager,
      std::unique_ptr<class VulkanContext> &vulkanContext);
  void recordRaytracingCommandBuffer(VkCommandBuffer commandBuffer,
                                     uint32_t imageIndex, uint8_t section);

  const VkPipeline &getPipeline() const;
  const VkPipelineLayout &getPipelineLayout() const;
  const VkDescriptorSet &getDescriptorSet(int i) const;
  SwapImage* getStorageImage();

private:

  using RaytracingPushConstants = ShaderPushConstants<PushConstant<RaytracingPushConstant, SHADER_RGEN>>;
  using RaytracingShaderGroup = ShaderGroup<RaytracingPushConstants, main_rmiss, main_rgen>;
  using RaytracingResourceSet = ShaderResourceSet<
    ResourceBinding<SwapImage, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, SHADER_RGEN, 0, 1>,
    ResourceBinding<SwapImage, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SHADER_RGEN, 1, 1>,
    ResourceBinding<SwapImage, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, SHADER_RGEN, 2, 1>,
    ResourceBinding<StagedSharedImage, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, SHADER_RGEN, 3, 512>,
    ResourceBinding<StagedSharedImage, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, SHADER_RGEN, 4, 1>,
    ResourceBinding<SwapImage, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, SHADER_RGEN, 5, 1>,
    ResourceBinding<SwapImage, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, SHADER_RGEN, 6, 1>,
    ResourceBinding<SwapImage, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, SHADER_RGEN, 7, 1>,
    ResourceBinding<SwapImage, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, SHADER_RGEN, 8, 1>
  >;
  using Pipeline = RaytracingPipeline<RaytracingShaderGroup, RaytracingResourceSet>;
  using RaytracingPushConstantData = PushConstantData<RaytracingPushConstant>;
  using RaytracingRenderPass_t = RaytracingRenderPass<RaytracingRenderPassPipeline<RaytracingPushConstantData, Pipeline>>;

  SwapImage raytracingStorageImage;
  SwapImage raytracingPositionStorageImage;
  SwapImage raytracingLightStorageImageX;
  SwapImage raytracingLightStorageImageY;
  SwapImage raytracingLightStorageImageZ;
  SwapImage raytracingLightStorageImageW;

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