#pragma once

#include <vulkan/vulkan_core.h>
class BindResource
{
    virtual void write(VkWriteDescriptorSet& descriptorWrite, int frame) = 0;
};
