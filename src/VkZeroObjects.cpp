
#include "Camera.hpp"
#include "VkZero/render_pass.hpp"
#include "VkZero/resource_manager.hpp"
#include "VoxelWorld.hpp"
#include "VkZero/image.hpp"
#include <memory>
#include <stdexcept>
#include <vulkan/vulkan_core.h>
#include "Engine.hpp"
#include "VkZeroObjects.hpp"
#include "VkZero/window.hpp"
#include "shaders.hpp"

#define RAYTRACE_HEIGHT 1080
#define RAYTRACE_WIDTH 1920

VkZeroObjects::VkZeroObjects(
                     std::unique_ptr<VoxelWorld> &voxelWorld,
                     std::unique_ptr<Camera> &camera, std::unique_ptr<VkZero::Window> &window, std::function<void(VkCommandBuffer, uint32_t)> after) :
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
    raytracingShaderGroup(raytracingPushConstants, VoxelEngine::get_shader<"main_rmiss">(), VoxelEngine::get_shader<"main_rgen">()),
    raytracingResourceSet{
      {&raytracingStorageImage},
      {&raytracingStorageImage},
      {&raytracingPositionStorageImage},
      {voxelWorld->voxelImages.data()},
      {&voxelWorld->voxelChunkMapImage},
      {&raytracingLightStorageImageX},
      {&raytracingLightStorageImageY},
      {&raytracingLightStorageImageZ},
      {&raytracingLightStorageImageW}
    },
    raytracingPipeline(RAYTRACE_WIDTH, RAYTRACE_HEIGHT, raytracingShaderGroup, raytracingResourceSet),
    raytracingRenderPass(RaytracingRenderPass_t::NO_FUNCTION, {raytracingPipeline, raytracingPushConstantData}, after),
    graphicsShaderGroup(graphicsPushConstants, VoxelEngine::get_shader<"main_vert">(), VoxelEngine::get_shader<"main_frag">()),
    graphicsResourceSet{{&raytracingStorageImage}},
    graphicsPipeline(graphicsShaderGroup, graphicsResourceSet),
    graphicsRenderpassAttachmentSet{{&window->getSwapChainImages()}},
    graphicsRenderPass{window->getSwapChainExtent().width, window->getSwapChainExtent().height, graphicsRenderpassAttachmentSet, graphicsPipeline},
    frame{raytracingRenderPass, graphicsRenderPass, window->impl}
{
  createRaytracingPipeline(VkZero::vkZero_core->device, camera->uniformBuffer,
                           voxelWorld->voxelImages.data(),
                           &voxelWorld->voxelChunkMapImage);
}

VkZeroObjects::~VkZeroObjects() {}

void VkZeroObjects::createRaytracingPipeline(
    VkDevice device, std::vector<VkBuffer> &uniformBuffer,
    VkZero::StagedSharedImage* voxelImage,
    VkZero::StagedSharedImage* voxelChunkMapImage) {

    for (int i = 0; i < 2; i++)
    {

      VkDescriptorBufferInfo bufferInfo{};
      bufferInfo.buffer = uniformBuffer[i];
      bufferInfo.offset = 0;
      bufferInfo.range = sizeof(TransformUBO);
  
      VkWriteDescriptorSet writeTransformDescriptorSet = {};
      writeTransformDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      writeTransformDescriptorSet.dstSet = raytracingResourceSet.impl->descriptorSets[i];
      writeTransformDescriptorSet.dstBinding = 1;
      writeTransformDescriptorSet.dstArrayElement = 0;
      writeTransformDescriptorSet.descriptorType =
          VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      writeTransformDescriptorSet.descriptorCount = 1;
      writeTransformDescriptorSet.pBufferInfo = &bufferInfo;
      

      vkUpdateDescriptorSets(VkZero::vkZero_core->device, 1, &writeTransformDescriptorSet, 0, nullptr);
    }
}

void VkZeroObjects::recordRaytracingCommandBuffer(VkCommandBuffer commandBuffer,
                                              uint32_t imageIndex,
                                              uint8_t section) {}

void VkZeroObjects::draw()
{
  frame.draw();
}
