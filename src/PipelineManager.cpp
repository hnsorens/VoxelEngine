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

PipelineManager::PipelineManager(std::unique_ptr<VulkanContext> &vulkanContext,
                                 std::unique_ptr<Raytracer> &raytracer) 
  {

    auto& vert_shader = VoxelEngine::get_shader<"main_vert">();
    auto& frag_shader = VoxelEngine::get_shader<"main_frag">();

    Image* image = new Image{RAYTRACE_WIDTH, RAYTRACE_HEIGHT,
        VK_FORMAT_R16G16B16A16_UNORM, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_LAYOUT_GENERAL};

    ShaderGroup group(
      vert_shader, frag_shader
    );

    ShaderResourceSet set1{vulkanContext,
      ResourceBinding<Image, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, SHADER_FRAGMENT, 1, 1>{image}
    };

    GraphicsPipeline something{
      vulkanContext,
      group, set1
    };





    while(1);

  DescriptorSetBuilder descriptorSetBuilder{};

  descriptorSetBuilder.addDescriptor(0, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
  descriptorSet = descriptorSetBuilder.build({ vulkanContext, MAX_FRAMES_IN_FLIGHT });
                                
  PipelineBuilder pipelineBuilder;

  // pipelineBuilder.addShader(new Shader{vulkanContext, "bin/vert.spv", VK_SHADER_STAGE_VERTEX_BIT});
  // pipelineBuilder.addShader(new Shader{vulkanContext, "bin/frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT});

  pipelineBuilder.addComponent(PipelineBuilder::VERTEX_INPUT_STATE
                            | PipelineBuilder::INPUT_ASSEMBLY_STATE
                            | PipelineBuilder::VIEWPORT_STATE
                            | PipelineBuilder::RASTERIZATION_STATE
                            | PipelineBuilder::MULTISAMPLE_STATE
                            | PipelineBuilder::COLOR_BLEND_STATE
                            | PipelineBuilder::DYNAMIC_STATE);

  pipeline = pipelineBuilder.build({ vulkanContext, vulkanContext->getRenderPass(), descriptorSet });

  imageSampler.resize(MAX_FRAMES_IN_FLIGHT);
  
  for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
  {
    VkSamplerCreateInfo samplerCreateInfo = {};
    samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
    samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
    samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_TRANSPARENT_BLACK;
    samplerCreateInfo.anisotropyEnable = VK_FALSE;
    samplerCreateInfo.maxAnisotropy = 1.0f;
    samplerCreateInfo.compareEnable = VK_FALSE;
    samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerCreateInfo.minLod = 0.0f;
    samplerCreateInfo.maxLod = VK_LOD_CLAMP_NONE;
    samplerCreateInfo.mipLodBias = 0.0f;
  
    if (vkCreateSampler(vulkanContext->getDevice(), &samplerCreateInfo, nullptr,
                        &imageSampler[i]) != VK_SUCCESS) {
      throw std::runtime_error("failed to create sampler");
    }
  
    VkDescriptorImageInfo imageInfo = {};
    imageInfo.imageView = raytracer->getStorageImage(i);
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    imageInfo.sampler = imageSampler[i];
    descriptorSet->write(vulkanContext, 0, 0, i, &imageInfo);
  }
}

PipelineManager::~PipelineManager() {}

const VkPipeline &PipelineManager::getGraphicsPipeline() const {
  return pipeline->pipeline;
}
const VkPipelineLayout &PipelineManager::getGraphicsPipelineLayout() const {
  return pipeline->layout;
}
const VkDescriptorSet &PipelineManager::getDescriptorSet(int i) const {
  return descriptorSet->get(i);
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