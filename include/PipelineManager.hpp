#pragma once
#include "shaders.hpp"
#include <memory>
#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace VkZero
{
  class VulkanContext;
  class Window;
}

class PipelineManager {
public:
  PipelineManager(std::unique_ptr<VkZero::VulkanContext> &vulkanContext,
                  std::unique_ptr<class Raytracer> &raytracer, std::unique_ptr<VkZero::Window>& window);
  ~PipelineManager();

  void createGraphicsPipeline(VkDevice device, VkRenderPass renderPass,
                              std::unique_ptr<class Raytracer> &raytracer);
  const VkPipeline &getGraphicsPipeline() const;
  const VkPipelineLayout &getGraphicsPipelineLayout() const;
  const VkDescriptorSet &getDescriptorSet(int i) const;
  const VkRenderPass &getRenderPass() const;
  const VkFramebuffer getFrameBuffer(int i) const { return framebuffers[i]; }
  
  void recreateFramebuffers(std::unique_ptr<VkZero::VulkanContext>& vulkanContext, std::unique_ptr<VkZero::Window>& window);

  static VkShaderModule createShaderModule(VkDevice device,
                                           const std::vector<char> &code);
  using PipelineShaderPushConstant = VkZero::ShaderPushConstants<>;
  using PipelineShaderGroup = VkZero::ShaderGroup<PipelineShaderPushConstant, main_vert, main_frag>;
  using PipelineShaderResources = VkZero::ShaderResourceSet<VkZero::ResourceBinding<VkZero::SwapImage, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VkZero::SHADER_FRAGMENT, 0, 1>>;
  using Pipeline = VkZero::GraphicsPipeline<PipelineShaderGroup, PipelineShaderResources>;
  using PipelineRenderPassResources = VkZero::RenderPassResourceSet<VkZero::RenderPassResource<"output">>;
  using PipelineRenderPass = VkZero::RenderPass<PipelineRenderPassResources, Pipeline>;

  PipelineShaderPushConstant pushConstants;
  PipelineShaderGroup group;
  PipelineShaderResources set1;
  Pipeline something;
  PipelineRenderPassResources set2;
  PipelineRenderPass renderPass;

  VkPipeline pipeline;
  VkPipelineLayout pipelineLayout;
  std::vector<VkDescriptorSet> descriptorSet;
  std::vector<VkFramebuffer> framebuffers;
  VkRenderPass renderpass;
};