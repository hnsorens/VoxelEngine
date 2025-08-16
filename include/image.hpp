#pragma once

#include "VulkanContext.hpp"
#include "BindResource.hpp"
#include <vulkan/vulkan_core.h>

class Image : BindResource
{
public:
    Image(uint32_t width, uint32_t height,
            VkFormat format, VkImageTiling tiling,
            VkImageUsageFlags usage,
            VkMemoryPropertyFlags properties,
            VkImageLayout initialLayout);

    virtual void write(VkWriteDescriptorSet& descriptorWrite, int frame) override
    {
        imageInfos[frame].imageLayout = imageLayout;
        imageInfos[frame].imageView = imageViews[frame];
        imageInfos[frame].sampler = sampler;
        descriptorWrite.pImageInfo = &imageInfos[frame];
    }

    VkDescriptorImageInfo imageInfos[MAX_FRAMES_IN_FLIGHT];

    VkImage images[MAX_FRAMES_IN_FLIGHT];
    VkImageView imageViews[MAX_FRAMES_IN_FLIGHT];
    VkDeviceMemory deviceMemories[MAX_FRAMES_IN_FLIGHT];
    VkSampler sampler;
    VkImageLayout imageLayout;
};