#include "Raytracer.hpp"
#include "Camera.hpp"
#include "CommandManager.hpp"
#include "PipelineManager.hpp"
#include "ResourceManager.hpp"
#include "VoxelWorld.hpp"
#include "VulkanContext.hpp"
#include "image.hpp"
#include <memory>
#include <stdexcept>
#include <vulkan/vulkan_core.h>
#include "Engine.hpp"
#include "shaders.hpp"

Raytracer::Raytracer(std::unique_ptr<CommandManager> &commandManager,
                     std::unique_ptr<VulkanContext> &vulkanContext,
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
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_LAYOUT_GENERAL}

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
SwapImage* Raytracer::getStorageImage() {
  return &raytracingStorageImage;
}

void Raytracer::createRaytracingResources(
    std::unique_ptr<CommandManager> &commandManager,
    std::unique_ptr<VulkanContext> &vulkanContext) {

  VkDevice device = vulkanContext->getDevice();
  VkPhysicalDevice physicalDevice = vulkanContext->getPhysicalDevice();
}

void Raytracer::createRaytracingPipeline(
    VkDevice device, std::vector<VkBuffer> &uniformBuffer,
    StagedSharedImage* voxelImage,
    StagedSharedImage* voxelChunkMapImage) {

    auto& rmiss_shader = VoxelEngine::get_shader<"main_rmiss">();
    auto& rgen_shader = VoxelEngine::get_shader<"main_rgen">();

    ShaderGroup group(
      rmiss_shader, rgen_shader
    );

    ShaderResourceSet set1{VoxelEngine::vulkanContext,
      ResourceBinding<SwapImage, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, SHADER_RGEN, 0, 1>{&raytracingStorageImage},
      ResourceBinding<SwapImage, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SHADER_RGEN, 1, 1>{getStorageImage()},
      ResourceBinding<SwapImage, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, SHADER_RGEN, 2, 1>{&raytracingPositionStorageImage},
      ResourceBinding<StagedSharedImage, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, SHADER_RGEN, 3, 512>{voxelImage},
      ResourceBinding<StagedSharedImage, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, SHADER_RGEN, 4, 1>{voxelChunkMapImage},
      ResourceBinding<SwapImage, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, SHADER_RGEN, 5, 1>{&raytracingLightStorageImageX},
      ResourceBinding<SwapImage, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, SHADER_RGEN, 6, 1>{&raytracingLightStorageImageY},
      ResourceBinding<SwapImage, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, SHADER_RGEN, 7, 1>{&raytracingLightStorageImageZ},
      ResourceBinding<SwapImage, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, SHADER_RGEN, 8, 1>{&raytracingLightStorageImageW}
    };

    RaytracingPipeline something{
      VoxelEngine::vulkanContext,
      group, set1
    };
    
}

void Raytracer::recordRaytracingCommandBuffer(VkCommandBuffer commandBuffer,
                                              uint32_t imageIndex,
                                              uint8_t section) {}