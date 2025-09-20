#include "Raytracer.hpp"
#include "Camera.hpp"
#include "CommandManager.hpp"
#include "PipelineManager.hpp"
#include "VkZero/render_pass.hpp"
#include "VkZero/resource_manager.hpp"
#include "VoxelWorld.hpp"
#include "VkZero/image.hpp"
#include <memory>
#include <stdexcept>
#include <vulkan/vulkan_core.h>
#include "Engine.hpp"
#include "shaders.hpp"

#define RAYTRACE_HEIGHT 1080
#define RAYTRACE_WIDTH 1920

Raytracer::Raytracer(std::unique_ptr<CommandManager> &commandManager,
                     std::unique_ptr<VoxelWorld> &voxelWorld,
                     std::unique_ptr<Camera> &camera) :
    raytracingStorageImage{RAYTRACE_WIDTH, RAYTRACE_HEIGHT, 1,
        VkZero::Format::R16G16B16A16Unorm,
        VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT},
    raytracingPositionStorageImage{RAYTRACE_WIDTH, RAYTRACE_HEIGHT, 1,
        VkZero::Format::R32Uint,
        VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT},

    raytracingLightStorageImageX{RAYTRACE_WIDTH, RAYTRACE_HEIGHT, 1,
        VkZero::Format::R32Uint,
        VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT},

    raytracingLightStorageImageY{RAYTRACE_WIDTH, RAYTRACE_HEIGHT, 1,
        VkZero::Format::R32Uint,
        VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT},

    raytracingLightStorageImageZ{RAYTRACE_WIDTH, RAYTRACE_HEIGHT, 1,
        VkZero::Format::R32Uint,
        VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT},

    raytracingLightStorageImageW{RAYTRACE_WIDTH, RAYTRACE_HEIGHT, 1,
        VkZero::Format::R32Uint,
        VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT},
    group(raytracingPushConstants, VoxelEngine::get_shader<"main_rmiss">(), VoxelEngine::get_shader<"main_rgen">()),
    set1{
      {&raytracingStorageImage},
      {getStorageImage()},
      {&raytracingPositionStorageImage},
      {voxelWorld->voxelImages.data()},
      {&voxelWorld->voxelChunkMapImage},
      {&raytracingLightStorageImageX},
      {&raytracingLightStorageImageY},
      {&raytracingLightStorageImageZ},
      {&raytracingLightStorageImageW}
    },
    something(RAYTRACE_WIDTH, RAYTRACE_HEIGHT, group, set1),
    renderPass({something, data})

{
  createRaytracingPipeline(VkZero::vkZero_core->device, camera->uniformBuffer,
                           voxelWorld->voxelImages.data(),
                           &voxelWorld->voxelChunkMapImage);
}

Raytracer::~Raytracer() {}

const VkPipeline &Raytracer::getPipeline() const { return raytracingPipeline; }
const VkPipelineLayout &Raytracer::getPipelineLayout() const {
  return raytracingPipelineLayout;
}
const VkDescriptorSet &Raytracer::getDescriptorSet(int i) const {
  return raytracingDescriptorSets[i];
}
VkZero::SwapImage* Raytracer::getStorageImage() {
  return &raytracingStorageImage;
}

void Raytracer::createRaytracingPipeline(
    VkDevice device, std::vector<VkBuffer> &uniformBuffer,
    VkZero::StagedSharedImage* voxelImage,
    VkZero::StagedSharedImage* voxelChunkMapImage) {

    raytracingPipeline = something.impl->pipeline;
    raytracingPipelineLayout = something.impl->pipelineLayout;
    raytracingDescriptorSetLayout = set1.impl->descriptorSetLayout;
    raytracingDescriptorSets = set1.impl->descriptorSets;
    for (int i = 0; i < 2; i++)
    {

      VkDescriptorBufferInfo bufferInfo{};
      bufferInfo.buffer = uniformBuffer[i];
      bufferInfo.offset = 0;
      bufferInfo.range = sizeof(TransformUBO);
  
      VkWriteDescriptorSet writeTransformDescriptorSet = {};
      writeTransformDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      writeTransformDescriptorSet.dstSet = raytracingDescriptorSets[i];
      writeTransformDescriptorSet.dstBinding = 1;
      writeTransformDescriptorSet.dstArrayElement = 0;
      writeTransformDescriptorSet.descriptorType =
          VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      writeTransformDescriptorSet.descriptorCount = 1;
      writeTransformDescriptorSet.pBufferInfo = &bufferInfo;
      

      vkUpdateDescriptorSets(VkZero::vkZero_core->device, 1, &writeTransformDescriptorSet, 0, nullptr);
    }
}

void Raytracer::recordRaytracingCommandBuffer(VkCommandBuffer commandBuffer,
                                              uint32_t imageIndex,
                                              uint8_t section) {}
