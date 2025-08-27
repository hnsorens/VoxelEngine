#pragma once
#include "VoxelWorld.hpp"
#include "image.hpp"
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
      StagedSwapImage* voxelImage,
      StagedSwapImage* voxelChunkMapImage);
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

  SwapImage raytracingStorageImage;
  SwapImage raytracingPositionStorageImage;
  SwapImage raytracingLightStorageImageX;
  SwapImage raytracingLightStorageImageY;
  SwapImage raytracingLightStorageImageZ;
  SwapImage raytracingLightStorageImageW;

  VkPipeline raytracingPipeline;
  VkPipelineLayout raytracingPipelineLayout;
  VkDescriptorSetLayout raytracingDescriptorSetLayout;
  std::vector<VkDescriptorSet> raytracingDescriptorSets;
};