#pragma once

#include "BindResource.hpp"
#include "VulkanInfo.hpp"
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <vector>
#include <vulkan/vulkan_core.h>

class ImageImpl
{
private:
    static void CreateImages(uint32_t width, uint32_t height, uint32_t depth,
        VkFormat format, VkImageTiling tiling,
        VkImageUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkImageLayout initialLayout, size_t imageCount, VkImage* images, VkImageView* imageViews, VkDeviceMemory* deviceMemories, VkSampler* sampler);

    static void CreateStagingBuffer(uint32_t width, uint32_t height, uint32_t depth, VkFormat format, VkBuffer* voxelStagingBuffer, VkDeviceMemory* voxelStagingBufferMemory, size_t imageCount);
    static void Write(uint32_t imageIndex, uint32_t width, uint32_t height, uint32_t depth, VkFormat format, VkDeviceMemory stagingBufferMemory, VkBuffer stagingBuffer, VkImage image, char* data);
    static void Write(VkCommandBuffer commandBuffer, uint32_t imageIndex, uint32_t width, uint32_t height, uint32_t depth, VkFormat format, VkDeviceMemory stagingBufferMemory, VkBuffer stagingBuffer, VkImage image, char* data);
    static void changeLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, int mipLevels);
    static void changeLayout(VkCommandBuffer commandBuffer, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, int mipLevels);

    static size_t formatSize(VkFormat format)
    {
        switch (format) {
            case VK_FORMAT_R32_UINT:
                return 4;
            case VK_FORMAT_R16G16B16A16_UNORM:
                return 8;
            case VK_FORMAT_R16_UINT:
                return 2;
            case VK_FORMAT_R8_UINT:
                return 1;
            default:
                return 1;
        }
    }

    template <int MaxImageCount>
    friend class ImageBase;

    template <int MaxImageCount>
    friend class StagedImageBase;
};

template <int MaxImageCount>
class ImageBase : BindResource
{
public:
    ImageBase(uint32_t width, uint32_t height, uint32_t depth,
            VkFormat format, VkImageTiling tiling,
            VkImageUsageFlags usage,
            VkMemoryPropertyFlags properties,
            VkImageLayout initialLayout) : width(width), height(height), depth(depth), format(format), imageLayout(initialLayout)
    {
        ImageImpl::CreateImages(width, height, depth, format, tiling, usage, properties, initialLayout, MaxImageCount, images, imageViews, deviceMemories, &sampler);   
    }

    ImageBase(std::vector<VkImage>& attachmentImages, std::vector<VkImageView>& attachmentImageViews)
    {
        for (int i = 0; i < attachmentImages.size(); i++)
        {
            images[i] = attachmentImages[i];
        }
        for (int i = 0; i < attachmentImageViews.size(); i++)
        {
            imageViews[i] = attachmentImageViews[i];
        }
    }

    void writeDescriptor(VkWriteDescriptorSet& descriptorWrite, int frame) override
    {
        frame = std::min(frame, maxImages);
        imageInfos[frame].imageLayout = imageLayout;
        imageInfos[frame].imageView = imageViews[frame];
        imageInfos[frame].sampler = sampler;
        descriptorWrite.pImageInfo = &imageInfos[frame];
    }

    void changeLayout(VkImageLayout newLayout, uint32_t index)
    {
        ImageImpl::changeLayout(images[index], format, imageLayout, newLayout, 1);
    }

    void changeLayout(VkCommandBuffer commandBuffer, VkImageLayout newLayout, uint32_t index)
    {
        ImageImpl::changeLayout(commandBuffer, images[index], format, imageLayout, newLayout, 1);
    }

    VkDescriptorImageInfo imageInfos[MaxImageCount];

    VkImage images[MaxImageCount];
    VkImageView imageViews[MaxImageCount];
    VkDeviceMemory deviceMemories[MaxImageCount];
    VkSampler sampler;
    VkImageLayout imageLayout;
    VkFormat format;

    uint32_t width, height, depth;

    static constexpr int maxImages = MaxImageCount;
};

template <int MaxImageCount>
class StagedImageBase : BindResource
{
public:
    StagedImageBase(uint32_t width, uint32_t height, uint32_t depth,
            VkFormat format, VkImageTiling tiling,
            VkImageUsageFlags usage,
            VkMemoryPropertyFlags properties,
            VkImageLayout initialLayout) : width(width), height(height), depth(depth), format(format), imageLayout(initialLayout)
    {
        ImageImpl::CreateImages(width, height, depth, format, tiling, usage, properties, initialLayout, MaxImageCount, images, imageViews, deviceMemories, &sampler);   
        ImageImpl::CreateStagingBuffer(width, height, depth, format, stagingBuffers, stagingBuffersMemory, MaxImageCount);
    }

    void changeLayout(VkImageLayout newLayout, uint32_t index)
    {
        ImageImpl::changeLayout(images[index], format, imageLayout, newLayout, 1);
    }

    void changeLayout(VkCommandBuffer commandBuffer, VkImageLayout newLayout, uint32_t index)
    {
        ImageImpl::changeLayout(commandBuffer, images[index], format, imageLayout, newLayout, 1);
    }

    void writeDescriptor(VkWriteDescriptorSet& descriptorWrite, int frame) override
    {
        frame = std::min(frame, maxImages-1);
        imageInfos[frame].imageLayout = imageLayout;
        imageInfos[frame].imageView = imageViews[frame];
        imageInfos[frame].sampler = sampler;
        descriptorWrite.pImageInfo = &imageInfos[frame];
    }

    void write(char* data, uint32_t index)
    {
        ImageImpl::Write(index, width, height, depth, format, stagingBuffersMemory[index], stagingBuffers[index], images[index], data);
    }

    void write(VkCommandBuffer commandBuffer, char* data, uint32_t index)
    {
        ImageImpl::Write(commandBuffer, index, width, height, depth, format, stagingBuffersMemory[index], stagingBuffers[index], images[index], data);
    }

    VkDescriptorImageInfo imageInfos[MaxImageCount];

    VkImage images[MaxImageCount];
    VkImageView imageViews[MaxImageCount];
    VkDeviceMemory deviceMemories[MaxImageCount];
    VkBuffer stagingBuffers[MaxImageCount];
    VkDeviceMemory stagingBuffersMemory[MaxImageCount];
    VkSampler sampler;
    VkImageLayout imageLayout;
    VkFormat format;

    uint32_t width, height, depth;

    static constexpr int maxImages = MaxImageCount;
};

class SwapImage : public ImageBase<MAX_FRAMES_IN_FLIGHT>
{
public:
    SwapImage(uint32_t width, uint32_t height, uint32_t depth,
            VkFormat format, VkImageTiling tiling,
            VkImageUsageFlags usage,
            VkMemoryPropertyFlags properties,
            VkImageLayout initialLayout) : 
    ImageBase(width, height, depth, format, tiling, usage, properties, initialLayout) {}
};

class SharedImage : public ImageBase<1>
{
public:
    SharedImage(uint32_t width, uint32_t height, uint32_t depth,
            VkFormat format, VkImageTiling tiling,
            VkImageUsageFlags usage,
            VkMemoryPropertyFlags properties,
            VkImageLayout initialLayout) : 
    ImageBase(width, height, depth, format, tiling, usage, properties, initialLayout) {}
};

class StagedSwapImage : public StagedImageBase<MAX_FRAMES_IN_FLIGHT>
{
public:
    StagedSwapImage(uint32_t width, uint32_t height, uint32_t depth,
            VkFormat format, VkImageTiling tiling,
            VkImageUsageFlags usage,
            VkMemoryPropertyFlags properties,
            VkImageLayout initialLayout) : 
    StagedImageBase(width, height, depth, format, tiling, usage, properties, initialLayout) {}
};

class StagedSharedImage : public StagedImageBase<1>
{
public:
    StagedSharedImage(uint32_t width, uint32_t height, uint32_t depth,
            VkFormat format, VkImageTiling tiling,
            VkImageUsageFlags usage,
            VkMemoryPropertyFlags properties,
            VkImageLayout initialLayout) : 
    StagedImageBase(width, height, depth, format, tiling, usage, properties, initialLayout) {}
};

class AttachmentImage : public ImageBase<MAX_FRAMEBUFFER_COUNT>
{
public:
    AttachmentImage(uint32_t width, uint32_t height, uint32_t depth,
            VkFormat format, VkImageTiling tiling,
            VkImageUsageFlags usage,
            VkMemoryPropertyFlags properties,
            VkImageLayout initialLayout) : 
    ImageBase(width, height, depth, format, tiling, usage, properties, initialLayout) {}
private:
    AttachmentImage(std::vector<VkImage>& images, std::vector<VkImageView>& imageViews) : ImageBase(images, imageViews) {}

    friend class WindowManager;
};

class StagedAttachmentImage : public StagedImageBase<MAX_FRAMEBUFFER_COUNT>
{
public:
    StagedAttachmentImage(uint32_t width, uint32_t height, uint32_t depth,
            VkFormat format, VkImageTiling tiling,
            VkImageUsageFlags usage,
            VkMemoryPropertyFlags properties,
            VkImageLayout initialLayout) : 
    StagedImageBase(width, height, depth, format, tiling, usage, properties, initialLayout) {}
};