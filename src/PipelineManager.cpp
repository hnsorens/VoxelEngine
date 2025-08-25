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

    Image swapchainImages{RAYTRACE_WIDTH, RAYTRACE_HEIGHT,
        VK_FORMAT_R16G16B16A16_UNORM, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_LAYOUT_GENERAL};

        
    swapchainImages.imageViews[0] = vulkanContext->getSwapChainImageViews()[0];
    swapchainImages.imageViews[1] = vulkanContext->getSwapChainImageViews()[1];

    RenderPassResourceSet set2{
      RenderPassResource<"output">{&swapchainImages}
    };

    auto extent = vulkanContext->getSwapChainExtent();

    RenderPass renderPass{
      extent.width,
      extent.height,
      vulkanContext,
      set2,
      something
    };

    printf("pipeline: %d\n", something.pipeline);
    pipeline = something.pipeline;
    pipelineLayout = something.pipelineLayout;
    renderpass = renderPass.renderPass;
    framebuffers = renderPass.framebuffers;
    descriptorSet = set1.descriptorSets;

        for (auto& s : framebuffers)
        {
            printf("frame: %d\n", s);
            fflush(stdout);
        }
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

void PipelineManager::recreateFramebuffers(std::unique_ptr<VulkanContext>& vulkanContext) {
    // Clean up old framebuffers
    for (auto framebuffer : framebuffers) {
        vkDestroyFramebuffer(vulkanContext->getDevice(), framebuffer, nullptr);
    }
    
    // Create new framebuffers with updated swapchain image views
    framebuffers.resize(2); // Assuming 2 framebuffers as in the original code
    
    for (size_t i = 0; i < 2; i++) {
        VkImageView attachments[] = {vulkanContext->getSwapChainImageViews()[i]};
        
        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderpass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = vulkanContext->getSwapChainExtent().width;
        framebufferInfo.height = vulkanContext->getSwapChainExtent().height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(vulkanContext->getDevice(), &framebufferInfo, nullptr,
                                &framebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}


VkShaderModule PipelineManager::createShaderModule(VkDevice device,
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