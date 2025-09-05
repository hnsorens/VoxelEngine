#pragma once
#include <vulkan/vulkan_core.h>
namespace VkZero
{
    class BindResource
    {
    public:
        virtual void writeDescriptor(VkDevice device, VkDescriptorSet descriptorSet, uint32_t binding, uint32_t element, VkDescriptorType type, int frame) = 0;
    };
}