#pragma once
#include "pipeline.hpp"
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
  Pipeline* pipeline;
  DescriptorSet* descriptorSet;
  std::vector<VkSampler> imageSampler;
};