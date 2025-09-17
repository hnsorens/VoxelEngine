#include "VkZero/Internal/shader_group_internal.hpp"

using namespace VkZero;

ShaderPushConstantsImpl_T::ShaderPushConstantsImpl_T(
    std::vector<ShaderPushConstantData> pushConstants) {
  uint32_t currentOffset = 0;
  ranges.reserve(pushConstants.size());
  for (auto &c : pushConstants) {
    ranges.push_back({.stageFlags = c.shaderStages,
                      .offset = currentOffset,
                      .size = c.structureSize}),
        currentOffset += c.structureSize;
  }
}

ShaderPushConstantsBase::ShaderPushConstantsBase(
    std::vector<ShaderPushConstantData> data) {
  impl = new ShaderPushConstantsImpl_T(data);
}

ShaderGroupImpl::ShaderGroupImpl(std::vector<ShaderImpl_T *> shaders,
                                 ShaderPushConstantsImpl_T *pushConstantsImpl)
    : m_shaders([&]() {
        std::vector<VkPipelineShaderStageCreateInfo> shaderCreateInfos;
        for (auto &s : shaders) {
          shaderCreateInfos.push_back(s->shaderInfo);
        }
        return std::move(shaderCreateInfos);
      }()),
      m_ranges(pushConstantsImpl->ranges) {}

ShaderGroupBase::ShaderGroupBase(
    std::vector<struct ShaderImpl_T *> shaders,
    struct ShaderPushConstantsImpl_T *pushConstants) {
  impl = new ShaderGroupImpl(shaders, pushConstants);
}
