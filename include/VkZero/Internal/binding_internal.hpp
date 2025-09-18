#pragma once

#include "VkZero/binding.hpp"

namespace VkZero {
struct ResourceBindingImpl_T {
  ResourceBindingImpl_T(std::vector<BindResource *> resources,
                        VkDescriptorType type, uint32_t descriptorCount,
                        uint32_t binding, VkShaderStageFlags stages);

  std::vector<BindResource *> resources;
  VkDescriptorType type;
  VkShaderStageFlags stages;
  uint32_t descriptorCount;
  uint32_t binding;

  void writeAll(VkDevice device, std::vector<VkDescriptorSet> descriptorSets);

  void write(VkDevice device, VkDescriptorSet &descriptorSet, int element,
             int frame);
};
} // namespace VkZero
