#pragma once

#include <vulkan/vulkan_core.h>
class BindResource
{
    virtual void writeDescriptor(VkWriteDescriptorSet& descriptorWrite, int frame) = 0;
};
