#pragma once
#include "pipeline.hpp"
#include <memory>
#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

class PipelineManager {
public:
  PipelineManager(std::unique_ptr<class VulkanContext> &vulkanContext,
                  std::unique_ptr<class Raytracer> &raytracer, std::unique_ptr<class WindowManager>& window);
  ~PipelineManager();

  void createGraphicsPipeline(VkDevice device, VkRenderPass renderPass,
                              std::unique_ptr<class Raytracer> &raytracer);
  const VkPipeline &getGraphicsPipeline() const;
  const VkPipelineLayout &getGraphicsPipelineLayout() const;
  const VkDescriptorSet &getDescriptorSet(int i) const;
  const VkRenderPass &getRenderPass() const;
  const VkFramebuffer getFrameBuffer(int i) const { return framebuffers[i]; }
  
  void recreateFramebuffers(std::unique_ptr<VulkanContext>& vulkanContext, std::unique_ptr<WindowManager>& window);

  static VkShaderModule createShaderModule(VkDevice device,
                                           const std::vector<char> &code);

private:

  using PipelineShaderPushConstant = ShaderPushConstants<>;
  using PipelineShaderGroup = ShaderGroup<PipelineShaderPushConstant, main_vert, main_frag>;
  using PipelineShaderResources = ShaderResourceSet<ResourceBinding<SwapImage, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, SHADER_FRAGMENT, 0, 1>>;
  using Pipeline = GraphicsPipeline<PipelineShaderGroup, PipelineShaderResources>;
  using PipelineRenderPassResources = RenderPassResourceSet<RenderPassResource<"output">>;
  using PipelineRenderPass = RenderPass<PipelineRenderPassResources, Pipeline>;

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