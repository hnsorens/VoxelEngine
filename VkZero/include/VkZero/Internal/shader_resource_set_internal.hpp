#pragma once

#include "VkZero/shader_resource_set.hpp"

namespace VkZero {

struct ShaderResourceSetImpl_T {
public:
  ShaderResourceSetImpl_T(std::vector<ResourceBindingImpl_T *> bindings);

  std::vector<ResourceBindingBase *> bindings;
  VkDescriptorSetLayout descriptorSetLayout;
  std::vector<VkDescriptorSet> descriptorSets;
};

} // namespace VkZero
