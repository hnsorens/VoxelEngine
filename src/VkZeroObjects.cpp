
#include "VkZeroObjects.hpp"
#include "Camera.hpp"
#include "Engine.hpp"
#include "VkZero/Internal/uniform_buffer_internal.hpp"
#include "VkZero/image.hpp"
#include "VkZero/render_pass.hpp"
#include "VkZero/Internal/resource_manager_internal.hpp"
#include "VkZero/window.hpp"
#include "VoxelWorld.hpp"
#include "shaders.hpp"
#include <memory>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

#define RAYTRACE_HEIGHT 1080
#define RAYTRACE_WIDTH 1920

VkZeroObjects::VkZeroObjects(
    std::unique_ptr<VoxelWorld> &voxelWorld, std::unique_ptr<Camera> &camera,
    std::unique_ptr<VkZero::Window> &window,
    std::function<void(VkCommandBuffer, uint32_t)> after)
    : raytracingStorageImage{RAYTRACE_WIDTH,
                             RAYTRACE_HEIGHT,
                             1,
                             VkZero::Format::R16G16B16A16Unorm,
                             VK_IMAGE_USAGE_STORAGE_BIT |
                                 VK_IMAGE_USAGE_SAMPLED_BIT,
                             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT},
      raytracingPositionStorageImage{RAYTRACE_WIDTH,
                                     RAYTRACE_HEIGHT,
                                     1,
                                     VkZero::Format::R32Uint,
                                     VK_IMAGE_USAGE_STORAGE_BIT |
                                         VK_IMAGE_USAGE_SAMPLED_BIT,
                                     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT},

      raytracingLightStorageImageX{RAYTRACE_WIDTH,
                                   RAYTRACE_HEIGHT,
                                   1,
                                   VkZero::Format::R32Uint,
                                   VK_IMAGE_USAGE_STORAGE_BIT |
                                       VK_IMAGE_USAGE_SAMPLED_BIT,
                                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT},

      raytracingLightStorageImageY{RAYTRACE_WIDTH,
                                   RAYTRACE_HEIGHT,
                                   1,
                                   VkZero::Format::R32Uint,
                                   VK_IMAGE_USAGE_STORAGE_BIT |
                                       VK_IMAGE_USAGE_SAMPLED_BIT,
                                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT},

      raytracingLightStorageImageZ{RAYTRACE_WIDTH,
                                   RAYTRACE_HEIGHT,
                                   1,
                                   VkZero::Format::R32Uint,
                                   VK_IMAGE_USAGE_STORAGE_BIT |
                                       VK_IMAGE_USAGE_SAMPLED_BIT,
                                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT},

      raytracingLightStorageImageW{RAYTRACE_WIDTH,
                                   RAYTRACE_HEIGHT,
                                   1,
                                   VkZero::Format::R32Uint,
                                   VK_IMAGE_USAGE_STORAGE_BIT |
                                       VK_IMAGE_USAGE_SAMPLED_BIT,
                                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT},
      raytracingShaderGroup(raytracingPushConstants,
                            VoxelEngine::get_shader<"main_rmiss">(),
                            VoxelEngine::get_shader<"main_rgen">()),
      raytracingResourceSet{
          {&raytracingStorageImage},         {&camera->uniformBuffer},
          {&raytracingPositionStorageImage}, {voxelWorld->voxelImages.data()},
          {&voxelWorld->voxelChunkMapImage}, {&raytracingLightStorageImageX},
          {&raytracingLightStorageImageY},   {&raytracingLightStorageImageZ},
          {&raytracingLightStorageImageW}},
      raytracingPipeline(RAYTRACE_WIDTH, RAYTRACE_HEIGHT, raytracingShaderGroup,
                         raytracingResourceSet),
      raytracingRenderPass(RaytracingRenderPass_t::NO_FUNCTION,
                           {raytracingPipeline, raytracingPushConstantData},
                           after),
      graphicsShaderGroup(graphicsPushConstants,
                          VoxelEngine::get_shader<"main_vert">(),
                          VoxelEngine::get_shader<"main_frag">()),
      graphicsResourceSet{{&raytracingStorageImage}},
      graphicsPipeline(graphicsShaderGroup, graphicsResourceSet),
      graphicsRenderpassAttachmentSet{{&window->getSwapChainImages()}},
      graphicsRenderPass{window->getSwapChainExtent().width,
                         window->getSwapChainExtent().height,
                         graphicsRenderpassAttachmentSet, graphicsPipeline},
      frame{raytracingRenderPass, graphicsRenderPass, window->impl} {
}

VkZeroObjects::~VkZeroObjects() {}

void VkZeroObjects::draw() { frame.draw(); }
