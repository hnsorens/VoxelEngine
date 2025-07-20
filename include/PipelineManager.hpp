#pragma once
#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

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

  static VkShaderModule createShaderModule(VkDevice device,
                                           const std::vector<char> &code);

private:
  VkPipeline graphicsPipeline;
  VkPipelineLayout graphicsPipelineLayout;
  VkDescriptorSetLayout descriptorSetLayout;
  VkDescriptorPool descriptorPool;
  std::vector<VkDescriptorSet> descriptorSets;
  std::vector<VkSampler> imageSampler;
};