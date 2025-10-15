
#include "VkZeroObjects.hpp"
#include "Camera.hpp"
#include "Engine.hpp"
#include "VkZero/image.hpp"
#include "VkZero/render_pass.hpp"
#include "VkZero/window.hpp"
#include "VoxelWorld.hpp"
#include "shaders.hpp"
#include <memory>
#include <stdexcept>

#define RAYTRACE_HEIGHT 1080
#define RAYTRACE_WIDTH 1920

VkZeroObjects::VkZeroObjects(
    VoxelWorld &voxelWorld, Camera &camera,
    VkZero::Window &window,
    std::function<void(void*, uint32_t)> after)
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
                            shaders.get<"main_rmiss">(),
                            shaders.get<"main_rgen">()),
      raytracingResourceSet{
          {&raytracingStorageImage},         {&camera.uniformBuffer},
          {&raytracingPositionStorageImage}, {voxelWorld.voxelImages.data()},
          {&voxelWorld.voxelChunkMapImage}, {&raytracingLightStorageImageX},
          {&raytracingLightStorageImageY},   {&raytracingLightStorageImageZ},
          {&raytracingLightStorageImageW}},
      raytracingPipeline(RAYTRACE_WIDTH, RAYTRACE_HEIGHT, raytracingShaderGroup,
                         raytracingResourceSet),
      raytracingRenderPass(RaytracingRenderPass_t::NO_FUNCTION,
                           {raytracingPipeline, raytracingPushConstantData},
                           after),
      graphicsShaderGroup(graphicsPushConstants,
                          shaders.get<"main_vert">(),
                          shaders.get<"main_frag">()),
      graphicsResourceSet{{&raytracingStorageImage}},
      graphicsPipeline(graphicsShaderGroup, graphicsResourceSet),
      graphicsRenderpassAttachmentSet{{&window.getSwapChainImages()}},
      graphicsRenderPass{window.getWidth(),
                         window.getHeight(),
                         graphicsRenderpassAttachmentSet, graphicsPipeline},
      frame{raytracingRenderPass, graphicsRenderPass, window.impl} {
}

VkZeroObjects::~VkZeroObjects() {}

void VkZeroObjects::draw() { frame.draw(); }
