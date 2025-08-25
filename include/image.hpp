#pragma once

#include "BindResource.hpp"
#include "VulkanInfo.hpp"
#include <vulkan/vulkan_core.h>

class ImageImpl
{
private:
    static void CreateImages(uint32_t width, uint32_t height,
        VkFormat format, VkImageTiling tiling,
        VkImageUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkImageLayout initialLayout, size_t imageCount, VkImage* images, VkImageView* imageViews, VkDeviceMemory* deviceMemories, VkSampler* sampler);

    template <int MaxImageCount>
    friend class Image;
};

template <int MaxImageCount>
class Image : BindResource
{
public:
    Image(uint32_t width, uint32_t height,
            VkFormat format, VkImageTiling tiling,
            VkImageUsageFlags usage,
            VkMemoryPropertyFlags properties,
            VkImageLayout initialLayout)
    {
        ImageImpl::CreateImages(width, height, format, tiling, usage, properties, initialLayout, MaxImageCount, images, imageViews, deviceMemories, &sampler);   
    }

    virtual void write(VkWriteDescriptorSet& descriptorWrite, int frame) override
    {
        imageInfos[frame].imageLayout = imageLayout;
        imageInfos[frame].imageView = imageViews[frame];
        imageInfos[frame].sampler = sampler;
        descriptorWrite.pImageInfo = &imageInfos[frame];
    }

    VkDescriptorImageInfo imageInfos[MaxImageCount];

    VkImage images[MaxImageCount];
    VkImageView imageViews[MaxImageCount];
    VkDeviceMemory deviceMemories[MaxImageCount];
    VkSampler sampler;
    VkImageLayout imageLayout;

    static constexpr int maxImages = MaxImageCount;
};

class ShaderImage : public Image<MAX_FRAMES_IN_FLIGHT>
{
public:
    ShaderImage(uint32_t width, uint32_t height,
            VkFormat format, VkImageTiling tiling,
            VkImageUsageFlags usage,
            VkMemoryPropertyFlags properties,
            VkImageLayout initialLayout) : 
    Image(width, height, format, tiling, usage, properties, initialLayout) {}
};

class AttachmentImage : public Image<MAX_FRAMEBUFFER_COUNT>
{
public:
    AttachmentImage(uint32_t width, uint32_t height,
            VkFormat format, VkImageTiling tiling,
            VkImageUsageFlags usage,
            VkMemoryPropertyFlags properties,
            VkImageLayout initialLayout) : 
    Image(width, height, format, tiling, usage, properties, initialLayout) {}
};