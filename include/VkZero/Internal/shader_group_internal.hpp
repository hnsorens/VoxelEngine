#pragma once

#include "VkZero/shader_group.hpp"

namespace VkZero {

struct ShaderPushConstantsImpl_T
{
  ShaderPushConstantsImpl_T(std::vector<ShaderPushConstantData> pushConstants);

  std::vector<VkPushConstantRange> ranges;
};

struct ShaderGroupImpl {
public:
  ShaderGroupImpl(std::vector<ShaderImpl_T*> shaders,
                  ShaderPushConstantsImpl_T *pushConstantsImpl, std::vector<AttachmentImpl_T*> attachments);
    std::vector<VkPipelineShaderStageCreateInfo> m_shaders;
    std::vector<VkPushConstantRange> &m_ranges;
    std::vector<AttachmentImpl_T*> attachments;
  };

}
