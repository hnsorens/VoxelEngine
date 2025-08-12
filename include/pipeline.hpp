#pragma once

#include "ResourceManager.hpp"
#include "VulkanContext.hpp"
#include "builder.hpp"
#include <GLFW/glfw3.h>

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>

class DescriptorSet
{
public:

  void write(std::unique_ptr<VulkanContext>& ctx, int binding, int element, int frame, VkDescriptorImageInfo* imageWrite);

  VkDescriptorSet& get(int frame)
  {
    return descriptorSets[frame];
  }
private:

  VkWriteDescriptorSet createDescriptorWrite(int binding, int element, int frame, VkDescriptorType type);

  std::vector<VkDescriptorSet> descriptorSets;
  VkDescriptorSetLayout descriptorSetLayout;
  VkDescriptorPool descriptorPool;
  std::vector<VkDescriptorSetLayout> setLayouts;
  VkDescriptorSetLayoutCreateInfo layoutCreateInfo;

  friend class DescriptorSetBuilder;
  friend class PipelineBuilder;
};

struct DescriptorSetInfo {
  std::unique_ptr<VulkanContext>& ctx;
  int framesInFlight;
};

class DescriptorSetBuilder : Builder<DescriptorSet, DescriptorSetInfo>
{
public:
  DescriptorSetBuilder() = default;
  void addDescriptor(int binding, int descriptorCount, VkDescriptorType descriptorType, VkShaderStageFlagBits stageFlags);
  DescriptorSet* build(const DescriptorSetInfo& info) override;

private:
  std::vector<VkDescriptorSetLayoutBinding> descriptorBindings;

  friend class PipelineBuilder;
};


struct Shader
{
  Shader(std::unique_ptr<VulkanContext>& ctx, std::string path, VkShaderStageFlagBits shaderStages)
  {
    auto shaderCode = ResourceManager::readFile(path);

    shaderModule = createShaderModule(ctx->getDevice(), shaderCode);

    shaderInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderInfo.stage = shaderStages;
    shaderInfo.module = shaderModule;
    shaderInfo.pName = "main";
    shaderInfo.flags = 0;
  }

private:

  VkShaderModule createShaderModule(VkDevice device,
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

  VkShaderModule shaderModule;
  VkPipelineShaderStageCreateInfo shaderInfo {};

  friend class PipelineBuilder;
};

struct Pipeline
{
public:
    VkPipeline pipeline;
    VkPipelineLayout layout;
private:
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;
    std::vector<VkSampler> imageSampler;
};

struct PipelineBuildInfo
{
  std::unique_ptr<VulkanContext>& ctx;
  VkRenderPass renderPass;
  DescriptorSet* descriptorSet;
};

class PipelineBuilder : Builder<Pipeline, PipelineBuildInfo>
{
public:
    void addShader(Shader* shader);

    enum PipelineComponents
    {
        VERTEX_INPUT_STATE = 1,
        INPUT_ASSEMBLY_STATE = 2,
        VIEWPORT_STATE = 4,
        RASTERIZATION_STATE = 8,
        MULTISAMPLE_STATE = 16,
        COLOR_BLEND_STATE = 32,
        DYNAMIC_STATE = 64,
    };
    void addComponent(int components);
    
    Pipeline* build(const PipelineBuildInfo& info) override;
    
  private:
    
    std::vector<VkPipelineShaderStageCreateInfo> shaders;
    VkGraphicsPipelineCreateInfo pipelineInfo{};

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    VkPipelineViewportStateCreateInfo viewportState{};
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    VkPipelineMultisampleStateCreateInfo multisampling{};
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    VkPipelineColorBlendStateCreateInfo colorBlending{};
    VkPipelineDynamicStateCreateInfo dynamicState{};

    std::vector<VkDynamicState> dynamicStates;
};
