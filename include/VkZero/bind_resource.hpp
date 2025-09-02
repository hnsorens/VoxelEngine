#pragma once
#include <vulkan/vulkan_core.h>

namespace VkZero
{
    class BindResource
    {
        virtual void writeDescriptor(VkWriteDescriptorSet& descriptorWrite, int frame) = 0;
    };
}