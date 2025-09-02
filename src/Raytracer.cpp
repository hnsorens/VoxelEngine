#include "Raytracer.hpp"
#include "Camera.hpp"
#include "CommandManager.hpp"
#include "PipelineManager.hpp"
#include "VkZero/render_pass.hpp"
#include "VkZero/resource_manager.hpp"
#include "VoxelWorld.hpp"
#include "VkZero/context.hpp"
#include "VkZero/image.hpp"
#include <memory>
#include <stdexcept>
#include <vulkan/vulkan_core.h>
#include "Engine.hpp"
#include "shaders.hpp"

Raytracer::Raytracer(std::unique_ptr<CommandManager> &commandManager,
                     std::unique_ptr<VkZero::VulkanContext> &vulkanContext,
                     std::unique_ptr<VoxelWorld> &voxelWorld,
                     std::unique_ptr<Camera> &camera) :
    raytracingStorageImage{RAYTRACE_WIDTH, RAYTRACE_HEIGHT, 1,
        VK_FORMAT_R16G16B16A16_UNORM, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_LAYOUT_GENERAL},
    raytracingPositionStorageImage{RAYTRACE_WIDTH, RAYTRACE_HEIGHT, 1,
        VK_FORMAT_R32_UINT, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_LAYOUT_GENERAL},

    raytracingLightStorageImageX{RAYTRACE_WIDTH, RAYTRACE_HEIGHT, 1,
        VK_FORMAT_R32_UINT, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_LAYOUT_GENERAL},

    raytracingLightStorageImageY{RAYTRACE_WIDTH, RAYTRACE_HEIGHT, 1,
        VK_FORMAT_R32_UINT, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_LAYOUT_GENERAL},

    raytracingLightStorageImageZ{RAYTRACE_WIDTH, RAYTRACE_HEIGHT, 1,
        VK_FORMAT_R32_UINT, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_LAYOUT_GENERAL},

    raytracingLightStorageImageW{RAYTRACE_WIDTH, RAYTRACE_HEIGHT, 1,
        VK_FORMAT_R32_UINT, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_LAYOUT_GENERAL},
    group(raytracingPushConstants, VoxelEngine::get_shader<"main_rmiss">(), VoxelEngine::get_shader<"main_rgen">()),
    set1{
      vulkanContext,
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
    something(vulkanContext, group, set1),
    renderPass(vulkanContext, {something, data})

{
  createRaytracingResources(commandManager, vulkanContext);
  createRaytracingPipeline(vulkanContext->getDevice(), camera->uniformBuffer,
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

void Raytracer::createRaytracingResources(
    std::unique_ptr<CommandManager> &commandManager,
    std::unique_ptr<VkZero::VulkanContext> &vulkanContext) {

  VkDevice device = vulkanContext->getDevice();
  VkPhysicalDevice physicalDevice = vulkanContext->getPhysicalDevice();
}

void Raytracer::createRaytracingPipeline(
    VkDevice device, std::vector<VkBuffer> &uniformBuffer,
    VkZero::StagedSharedImage* voxelImage,
    VkZero::StagedSharedImage* voxelChunkMapImage) {

    // auto& rmiss_shader = VoxelEngine::get_shader<"main_rmiss">();
    // auto& rgen_shader = VoxelEngine::get_shader<"main_rgen">();

    // RaytracingPushConstants raytracingPushConstants;

    // RaytracingShaderGroup group(
    //   raytracingPushConstants,
    //   rmiss_shader, rgen_shader
    // );

    // RaytracingResourceSet set1{VoxelEngine::vulkanContext,
    //   {&raytracingStorageImage},
    //   {getStorageImage()},
    //   {&raytracingPositionStorageImage},
    //   {voxelImage},
    //   {voxelChunkMapImage},
    //   {&raytracingLightStorageImageX},
    //   {&raytracingLightStorageImageY},
    //   {&raytracingLightStorageImageZ},
    //   {&raytracingLightStorageImageW}
    // };

    // Pipeline something{
    //   VoxelEngine::vulkanContext,
    //   group, set1
    // };
    
    // RaytracingPushConstantData* data = new RaytracingPushConstantData;
    // RaytracingPushConstant* idk = data->get<RaytracingPushConstant>();

    // RaytracingRenderPass_t renderPass(
    //   VoxelEngine::vulkanContext,
    //   {something, data}
    // );
    
    raytracingPipeline = something.pipeline;
    printf("RAY PIPELINE : %d\n", something.pipeline); fflush(stdout);
    raytracingPipelineLayout = something.pipelineLayout;
    raytracingDescriptorSetLayout = set1.getLayout();
    raytracingDescriptorSets = set1.descriptorSets;
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
      

      vkUpdateDescriptorSets(VoxelEngine::vulkanContext->getDevice(), 1, &writeTransformDescriptorSet, 0, nullptr);
    }
}

void Raytracer::recordRaytracingCommandBuffer(VkCommandBuffer commandBuffer,
                                              uint32_t imageIndex,
                                              uint8_t section) {}