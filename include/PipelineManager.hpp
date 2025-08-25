#pragma once
#include "pipeline.hpp"
#include <memory>
#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

class PipelineManager {
public:
  PipelineManager(std::unique_ptr<class VulkanContext> &vulkanContext,
                  std::unique_ptr<class Raytracer> &raytracer);
  ~PipelineManager();

  void createGraphicsPipeline(VkDevice device, VkRenderPass renderPass,
                              std::unique_ptr<class Raytracer> &raytracer);
  const VkPipeline &getGraphicsPipeline() const;
  const VkPipelineLayout &getGraphicsPipelineLayout() const;
  const VkDescriptorSet &getDescriptorSet(int i) const;
  const VkRenderPass &getRenderPass() const;
  const VkFramebuffer getFrameBuffer(int i) const { return framebuffers[i]; }
  
  void recreateFramebuffers(std::unique_ptr<VulkanContext>& vulkanContext);

  static VkShaderModule createShaderModule(VkDevice device,
                                           const std::vector<char> &code);

private:
  VkPipeline pipeline;
  VkPipelineLayout pipelineLayout;
  std::vector<VkDescriptorSet> descriptorSet;
  std::vector<VkFramebuffer> framebuffers;
  VkRenderPass renderpass;
};