#include "pipeline.hpp"
#include <cstdio>


VkWriteDescriptorSet DescriptorSet::createDescriptorWrite(int binding, int element, int frame, VkDescriptorType type)
{
  VkWriteDescriptorSet writeDescriptorSet;
  writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  writeDescriptorSet.dstSet = descriptorSets[frame];
  writeDescriptorSet.dstBinding = binding;
  writeDescriptorSet.dstArrayElement = element;
  writeDescriptorSet.descriptorType = type;
  writeDescriptorSet.descriptorCount = 1;
  writeDescriptorSet.pNext = nullptr;
  return writeDescriptorSet;
}

void DescriptorSet::write(std::unique_ptr<VulkanContext>& ctx, int binding, int element, int frame, VkDescriptorImageInfo* imageWrite)
{
  VkWriteDescriptorSet writeDescriptorSet = createDescriptorWrite(binding, element, frame, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
  writeDescriptorSet.pImageInfo = imageWrite;

  vkUpdateDescriptorSets(ctx->getDevice(), 1, &writeDescriptorSet, 0, nullptr);
  // printf("UPDATE\n"); fflush(stdout);
}

void DescriptorSetBuilder::addDescriptor(int binding, int descriptorCount, VkDescriptorType descriptorType, VkShaderStageFlagBits stageFlags)
{
  // Make a new object for descriptors;
  VkDescriptorSetLayoutBinding bindInfo = {};
  bindInfo.binding = binding;
  bindInfo.descriptorCount = descriptorCount;
  bindInfo.descriptorType = descriptorType;
  bindInfo.stageFlags = stageFlags;
  bindInfo.pImmutableSamplers = nullptr;
  descriptorBindings.push_back(bindInfo);
}

DescriptorSet* DescriptorSetBuilder::build(const DescriptorSetInfo& info)
{

  DescriptorSet* descriptorSet = new DescriptorSet;

  descriptorSet->descriptorSets.resize(info.framesInFlight);

  descriptorSet->layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  descriptorSet->layoutCreateInfo.bindingCount = descriptorBindings.size();
  descriptorSet->layoutCreateInfo.pBindings = descriptorBindings.data();
  descriptorSet->layoutCreateInfo.pNext = 0;

  if (vkCreateDescriptorSetLayout(info.ctx->getDevice(), &descriptorSet->layoutCreateInfo, nullptr,
                                  &descriptorSet->descriptorSetLayout) != VK_SUCCESS) {
    throw std::runtime_error(
        "failed to create raytracing descriptor set layout!");
  }

  VkDescriptorPoolSize poolSize = {};
  poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  poolSize.descriptorCount = 2;

  VkDescriptorPoolCreateInfo poolCreateInfo = {};
  poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolCreateInfo.poolSizeCount = 1;
  poolCreateInfo.pPoolSizes = &poolSize;
  poolCreateInfo.maxSets = 50;

  if (vkCreateDescriptorPool(info.ctx->getDevice(), &poolCreateInfo, nullptr,
                             &descriptorSet->descriptorPool) != VK_SUCCESS) {
    throw std::runtime_error("failed to create raytracing descriptor pool!");
  }
  
  descriptorSet->setLayouts.resize(info.framesInFlight);
  for (int i = 0; i < info.framesInFlight; i++)
  {
    descriptorSet->setLayouts[i] = descriptorSet->descriptorSetLayout;
  }

  VkDescriptorSetAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = descriptorSet->descriptorPool;
  allocInfo.descriptorSetCount = static_cast<uint32_t>(descriptorSet->setLayouts.size());
  allocInfo.pSetLayouts = descriptorSet->setLayouts.data();

  if (vkAllocateDescriptorSets(info.ctx->getDevice(), &allocInfo, descriptorSet->descriptorSets.data())) {
    throw std::runtime_error("Failed to create raytracing descriptor set!");
  }
  
  return descriptorSet;
}

// void PipelineBuilder::addShader(Shader* shader)
// {
//   shaders.push_back(shader->shaderInfo);
// }

void PipelineBuilder::addComponent(int components)
{
  if (components & VERTEX_INPUT_STATE)
  {
    vertexInputInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
  }
  if (components & INPUT_ASSEMBLY_STATE)
  {
    inputAssembly.sType =
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
  }
  if (components & VIEWPORT_STATE)
  {
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;
    pipelineInfo.pViewportState = &viewportState;
  }
  if (components & RASTERIZATION_STATE)
  {
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    pipelineInfo.pRasterizationState = &rasterizer;
  }
  if (components & MULTISAMPLE_STATE)
  {
    multisampling.sType =
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    pipelineInfo.pMultisampleState = &multisampling;
  }
  if (components & COLOR_BLEND_STATE)
  {
    colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    colorBlending.sType =
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;
    pipelineInfo.pColorBlendState = &colorBlending;
  }
  if (components & DYNAMIC_STATE)
  {
    dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT,VK_DYNAMIC_STATE_SCISSOR};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();
    pipelineInfo.pDynamicState = &dynamicState;
  }
}

Pipeline* PipelineBuilder::build(const PipelineBuildInfo& info)
{
  Pipeline* pipeline = new Pipeline;

  /* Pipeline Layout */
  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 1;
  pipelineLayoutInfo.pushConstantRangeCount = 0;
  pipelineLayoutInfo.pSetLayouts = &info.descriptorSet->descriptorSetLayout;

  if (vkCreatePipelineLayout(info.ctx->getDevice(), &pipelineLayoutInfo, nullptr,
                             &pipeline->layout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create pipeline layout!");
  }

  /* Pipeline creation */
  printf("%s %d %d\n", shaders[0].pName, 0, 0);
  pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.stageCount = shaders.size();
  pipelineInfo.pStages = shaders.data();
  pipelineInfo.renderPass = info.renderPass;
  pipelineInfo.subpass = 0;
  pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
  pipelineInfo.layout = pipeline->layout;

  if (vkCreateGraphicsPipelines(info.ctx->getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo,
  nullptr, &pipeline->pipeline) != VK_SUCCESS) {
    throw std::runtime_error("failed to create graphics pipeline!");
    }

  return pipeline;
}