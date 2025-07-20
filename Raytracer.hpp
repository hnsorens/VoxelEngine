#pragma once
#include "VoxelWorld.hpp"
#include <memory>
#include <vector>
#define GLFW_INCLUDE_VULKAN
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
      std::vector<VkImageView> &voxelImageView, VkSampler voxelTextureSampler,
      std::vector<VkImageView> &voxelChunkMapImageView);
  void createRaytracingResources(
      std::unique_ptr<class CommandManager> &commandManager,
      std::unique_ptr<class VulkanContext> &vulkanContext);
  void recordRaytracingCommandBuffer(VkCommandBuffer commandBuffer,
                                     uint32_t imageIndex, uint8_t section);

  const VkPipeline &getPipeline() const;
  const VkPipelineLayout &getPipelineLayout() const;
  const VkDescriptorSet &getDescriptorSet(int i) const;
  const VkImageView &getStorageImage(int i) const;

private:
  std::vector<VkImage> raytracingStorageImage;
  std::vector<VkImageView> raytracingStorageImageView;
  std::vector<VkDeviceMemory> raytracingStorageMemory;

  std::vector<VkImage> raytracingPositionStorageImage;
  std::vector<VkImageView> raytracingPositionStorageImageView;
  std::vector<VkDeviceMemory> raytracingPositionStorageMemory;

  std::vector<VkImage> raytracingLightStorageImageX;
  std::vector<VkImageView> raytracingLightStorageImageViewX;
  std::vector<VkDeviceMemory> raytracingLightStorageMemoryX;

  std::vector<VkImage> raytracingLightStorageImageY;
  std::vector<VkImageView> raytracingLightStorageImageViewY;
  std::vector<VkDeviceMemory> raytracingLightStorageMemoryY;

  std::vector<VkImage> raytracingLightStorageImageZ;
  std::vector<VkImageView> raytracingLightStorageImageViewZ;
  std::vector<VkDeviceMemory> raytracingLightStorageMemoryZ;

  std::vector<VkImage> raytracingLightStorageImageW;
  std::vector<VkImageView> raytracingLightStorageImageViewW;
  std::vector<VkDeviceMemory> raytracingLightStorageMemoryW;

  VkPipeline raytracingPipeline;
  VkPipelineLayout raytracingPipelineLayout;
  VkDescriptorSetLayout raytracingDescriptorSetLayout;
  VkDescriptorPool raytracingDescriptorPool;
  std::vector<VkDescriptorSet> raytracingDescriptorSets;
};