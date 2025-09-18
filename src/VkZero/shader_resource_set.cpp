#include "VkZero/Internal/shader_resource_set_internal.hpp"
#include "VkZero/Internal/binding_internal.hpp"

using namespace VkZero;

ShaderResourceSetImpl_T::ShaderResourceSetImpl_T(
    std::vector<ResourceBindingImpl_T *> bindings)
    : descriptorSetLayout([&]() {
        std::vector<VkDescriptorSetLayoutBinding> descriptorBindings;

        for (auto &binding : bindings) {
          VkDescriptorSetLayoutBinding bindInfo{};
          bindInfo.binding = binding->binding;
          bindInfo.descriptorCount = binding->descriptorCount;
          bindInfo.descriptorType = binding->type;
          bindInfo.stageFlags = binding->stages;
          bindInfo.pImmutableSamplers = nullptr;
          descriptorBindings.push_back(bindInfo);
        }

        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
        descriptorSetLayoutInfo.sType =
            VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutInfo.bindingCount =
            static_cast<uint32_t>(descriptorBindings.size());
        descriptorSetLayoutInfo.pBindings = descriptorBindings.data();
        descriptorSetLayoutInfo.pNext = nullptr;

        descriptorSetLayoutInfo.flags =
            VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;

        VkDescriptorBindingFlags bindless_flags =
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT |
            VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT;
        std::vector<VkDescriptorBindingFlags> flags;

        for (auto &binding : bindings) {
          if (binding->descriptorCount > 1) {
            flags.push_back(bindless_flags);
          } else {
            flags.push_back(0);
          }
        }

        VkDescriptorSetLayoutBindingFlagsCreateInfoEXT extended_info{
            VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT,
            nullptr};
        extended_info.bindingCount = flags.size();
        extended_info.pBindingFlags = flags.data();

        descriptorSetLayoutInfo.pNext = &extended_info;

        VkDescriptorSetLayout layout;
        if (vkCreateDescriptorSetLayout(vkZero_core->device,
                                        &descriptorSetLayoutInfo, nullptr,
                                        &layout) != VK_SUCCESS) {
          throw std::runtime_error("failed to create descriptor set layout!");
        }

        return layout;
      }()),
      descriptorSets([&]() {
        VkDescriptorSetAllocateInfo allocInfo = {};
        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{
            MAX_FRAMES_IN_FLIGHT, descriptorSetLayout};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = DescriptorPool::instance().get();
        allocInfo.descriptorSetCount = MAX_FRAMES_IN_FLIGHT;
        allocInfo.pSetLayouts = descriptorSetLayouts.data();

        std::vector<VkDescriptorSet> descriptorSets(MAX_FRAMES_IN_FLIGHT);
        if (vkAllocateDescriptorSets(vkZero_core->device, &allocInfo,
                                     descriptorSets.data())) {
          throw std::runtime_error(
              "Failed to create raytracing descriptor set!");
        }

        return std::move(descriptorSets);
      }()) {
  for (auto &binding : bindings) {
    binding->writeAll(vkZero_core->device, descriptorSets);
  }
}

ShaderResourceSetBase::ShaderResourceSetBase(std::vector<ResourceBindingImpl_T *> bindings) {
  impl = new ShaderResourceSetImpl_T(bindings);
}
