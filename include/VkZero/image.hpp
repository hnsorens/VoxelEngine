#pragma once

#include "VkZero/bind_resource.hpp"
#include "VkZero/info.hpp"
#include "image.hpp"
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>
#include "VkZero/types.hpp"

namespace VkZero 
{
    class Image
    {
    protected:

        using StagingData = struct StagingData_T*;
        using ImageData = struct ImageData_T*;
        using ImageImpl = struct ImageImpl_T*;

        Image(VkImage* vk_images, VkImageView* imageViews, size_t imageCount, ImageData* images);
        Image(uint32_t width, uint32_t height, uint32_t depth,
            Format format,
            VkImageUsageFlags usage,
            VkMemoryPropertyFlags properties,
            ImageData* images, size_t imageCount);

        Image(uint32_t width, uint32_t height, uint32_t depth,
            Format format,
            VkImageUsageFlags usage,
            VkMemoryPropertyFlags properties,
            ImageData* images, StagingData* staging, size_t imageCount);

        void Write(ImageData image, StagingData staging, char* data);
        void Write(VkCommandBuffer commandBuffer, ImageData image, StagingData staging, char* data);
        void WriteDescriptor(VkDevice device, VkDescriptorSet descriptorSet, uint32_t binding, uint32_t element, VkDescriptorType type, ImageData image);
        void changeLayout(ImageData image, VkImageLayout newLayout, int mipLevels);
        void changeLayout(VkCommandBuffer commandBuffer, ImageData image, VkImageLayout newLayout, int mipLevels);

        ImageImpl impl;
    };

    template <int MaxImageCount>
    class ImageBase : public BindResource, Image
    {
    public:
        ImageBase(uint32_t width, uint32_t height, uint32_t depth,
                Format format,
                VkImageUsageFlags usage,
                VkMemoryPropertyFlags properties) :
                Image(width, height, depth, format, usage, properties, images, MaxImageCount) {}

        ImageBase(VkImage* vk_images, VkImageView* imageViews, size_t imageCount) :
                Image(vk_images, imageViews, imageCount, images) {}

        void writeDescriptor(VkDevice device, VkDescriptorSet descriptorSet, uint32_t binding, uint32_t element, VkDescriptorType type, int frame) override
        {
            frame = std::min(frame, maxImages-1);
            Image::WriteDescriptor(device, descriptorSet, binding, element, type, images[frame]);
        }

        void changeLayout(ImageLayout newLayout, uint32_t index)
        {
            Image::changeLayout(images[index], newLayout, 1);
        }

        void changeLayout(VkCommandBuffer commandBuffer, ImageLayout newLayout, uint32_t index)
        {
            Image::changeLayout(commandBuffer, images[index], newLayout, 1);
        }

        ImageData images[MaxImageCount];

        static constexpr int maxImages = MaxImageCount;
    };

    template <int MaxImageCount>
    class StagedImageBase : public BindResource, Image
    {
    public:
        StagedImageBase(uint32_t width, uint32_t height, uint32_t depth,
                Format format,
                VkImageUsageFlags usage,
                VkMemoryPropertyFlags properties) :
                Image(width, height, depth, format, usage, properties, images, staging, MaxImageCount)
        {}

        void changeLayout(ImageLayout newLayout, uint32_t index)
        {
            Image::changeLayout(images[index], (VkImageLayout)newLayout, 1);
        }

        void changeLayout(VkCommandBuffer commandBuffer, ImageLayout newLayout, uint32_t index)
        {
            Image::changeLayout(commandBuffer, images[index], (VkImageLayout)newLayout, 1);
        }

        void writeDescriptor(VkDevice device, VkDescriptorSet descriptorSet, uint32_t binding, uint32_t element, VkDescriptorType type, int frame) override
        {
            frame = std::min(frame, maxImages-1);
            Image::WriteDescriptor(device, descriptorSet, binding, element, type, images[frame]);
        }

        void write(char* data, uint32_t index) 
        { 
            Image::Write(images[index], staging[index], data); 
        }

        void write(VkCommandBuffer commandBuffer, char* data, uint32_t index)
        {
            Image::Write(commandBuffer, images[index], staging[index], data);
        }

        ImageData images[MaxImageCount];
        StagingData staging[MaxImageCount];

        static constexpr int maxImages = MaxImageCount;
    };

    using SwapImage = class ImageBase<2>;
    using SharedImage = class ImageBase<1>;
    using StagedSwapImage = class StagedImageBase<2>;
    using StagedSharedImage = class StagedImageBase<1>;
    using AttachmentImage = class ImageBase<3>;
    using StagedAttachmentImage = class StagedImageBase<3>;
}