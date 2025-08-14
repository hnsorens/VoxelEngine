#pragma once

#include "VulkanContext.hpp"
#include <vulkan/vulkan_core.h>

class Image
{
public:
    Image(uint32_t width, uint32_t height,
            VkFormat format, VkImageTiling tiling,
            VkImageUsageFlags usage,
            VkMemoryPropertyFlags properties,
            VkImageLayout initialLayout);

    VkImage images[MAX_FRAMES_IN_FLIGHT];
    VkImageView imageViews[MAX_FRAMES_IN_FLIGHT];
    VkDeviceMemory deviceMemories[MAX_FRAMES_IN_FLIGHT];
};