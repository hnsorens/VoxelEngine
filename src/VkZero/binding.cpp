#include "VkZero/Internal/binding_internal.hpp"
#include "VkZero/Internal/bind_resource_internal.hpp"

using namespace VkZero;

ResourceBindingImpl_T::ResourceBindingImpl_T(std::vector<BindResource*> resources,
                        VkDescriptorType type, uint32_t descriptorCount,
                        uint32_t binding, VkShaderStageFlags stages)
      : resources(resources), descriptorCount(descriptorCount), type(type),
        binding(binding), stages(stages) {}

  std::vector<BindResource *> resources;
  VkDescriptorType type;
  VkShaderStageFlags stages;
  uint32_t descriptorCount;
  uint32_t binding;
  
  void ResourceBindingImpl_T::writeAll(VkDevice device, std::vector<VkDescriptorSet> descriptorSets) {
    for (int i = 0; i < descriptorCount; i++) {
      for (int frame = 0; frame < descriptorSets.size(); frame++)
        write(device, descriptorSets[frame], i, frame);
    }
  }

  void ResourceBindingImpl_T::write(VkDevice device, VkDescriptorSet &descriptorSet, int element,
             int frame) {
    resources[element]->writeDescriptor(device, descriptorSet, binding, element,
                                        type, frame);
  }

ResourceBindingBase::ResourceBindingBase(std::vector<BindResource *> resources,
                                         VkDescriptorType type,
                                         uint32_t descriptorCount,
                                         uint32_t binding,
                                         VkShaderStageFlags stages) {
  impl = new ResourceBindingImpl_T(resources, type, descriptorCount, binding,
                                   stages);
}
