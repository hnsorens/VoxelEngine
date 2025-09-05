#pragma once

#include "VkZero/image.hpp"

namespace VkZero
{
    struct ImageData_T
    {
        VkImage image;
        VkImageView view;
        VkDeviceMemory memory;
    };

    struct StagingData_T
    {
        VkBuffer buffer;
        VkDeviceMemory memory;
    };

    struct ImageImpl_T
    {
        uint32_t width, height, depth;
        VkSampler sampler;
        VkImageLayout imageLayout;
        VkFormat format;
    };
}