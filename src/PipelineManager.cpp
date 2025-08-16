#include "shaders.hpp"
#include "Engine.hpp"
#include <PipelineManager.hpp>
#include <Raytracer.hpp>
#include <ResourceManager.hpp>
#include <VulkanContext.hpp>
#include <pipeline.hpp>
#include <memory>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>
#include "image.hpp"
#include "RenderPass.hpp"

PipelineManager::PipelineManager(std::unique_ptr<VulkanContext> &vulkanContext,
                                 std::unique_ptr<Raytracer> &raytracer) 
  {

    auto& vert_shader = VoxelEngine::get_shader<"main_vert">();
    auto& frag_shader = VoxelEngine::get_shader<"main_frag">();

    ShaderGroup group(
      vert_shader, frag_shader
    );

    ShaderResourceSet set1{vulkanContext,
      ResourceBinding<Image, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, SHADER_FRAGMENT, 0, 1>{raytracer->getStorageImage()}
    };

    GraphicsPipeline something{
      vulkanContext,
      group, set1
    };

    RenderPass renderPass{
      vulkanContext,
      something
    };

    printf("pipeline: %d\n", something.pipeline);
    pipeline = something.pipeline;
    pipelineLayout = something.pipelineLayout;
    renderpass = renderPass.renderPass;
    descriptorSet = set1.descriptorSets;
}

PipelineManager::~PipelineManager() {}

const VkPipeline &PipelineManager::getGraphicsPipeline() const {
  return pipeline;
}
const VkPipelineLayout &PipelineManager::getGraphicsPipelineLayout() const {
  return pipelineLayout;
}
const VkRenderPass &PipelineManager::getRenderPass() const {
  return renderpass;
}
const VkDescriptorSet &PipelineManager::getDescriptorSet(int i) const {
  return descriptorSet[i];
}


VkShaderModule
PipelineManager::createShaderModule(VkDevice device,
                                    const std::vector<char> &code) {
  VkShaderModuleCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = code.size();
  createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

  VkShaderModule shaderModule;
  if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create shader module!");
  }

  return shaderModule;
}
  std::vector<VkSampler> imageSampler;