#include "VkZero/window.hpp"
#include "shaders.hpp"
#include "Engine.hpp"
#include <PipelineManager.hpp>
#include <Raytracer.hpp>
#include <VkZero/resource_manager.hpp>
#include <memory>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>
#include "VkZero/image.hpp"
#include "VkZero/render_pass.hpp"
#include "VkZero/Internal/graphics_renderpass_internal.hpp"

PipelineManager::PipelineManager(std::unique_ptr<Raytracer> &raytracer,
                                std::unique_ptr<VkZero::Window>& window) : 
    group(pushConstants, VoxelEngine::get_shader<"main_vert">(), VoxelEngine::get_shader<"main_frag">()),
    set1{{raytracer->getStorageImage()}},
    something(group, set1),
    set2{{&window->getSwapChainImages()}},
    renderPass{window->getSwapChainExtent().width, window->getSwapChainExtent().height, set2, something}
  {
    
    // pipeline = something.pipeline;
    // pipelineLayout = something.pipelineLayout;
    // renderpass = renderPass.renderPass;
    // framebuffers = renderPass.framebuffers;
    // descriptorSet = set1.descriptorSets;

        for (auto& s : framebuffers)
        {
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

void PipelineManager::recreateFramebuffers(std::unique_ptr<VkZero::Window>& window) {
    renderPass.impl->recreateSwapchain(window);
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
