#pragma once

#include "VkZero/Internal/core_internal.hpp"
#include "VkZero/bind_resource.hpp"
#include "VkZero/info.hpp"
#include "VkZero/resource_manager.hpp"
#include "VkZero/types.hpp"
#include "image.hpp"
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace VkZero {
struct ImageData_T {
  VkImage image;
  VkImageView view;
  VkDeviceMemory memory;
};

struct StagingData_T {
  VkBuffer buffer;
  VkDeviceMemory memory;
};

struct ImageImpl_T {
  uint32_t width, height, depth;
  VkSampler sampler;
  VkImageLayout imageLayout;
  VkFormat format;
};
class Image {
protected:
  Image(VkImage *images, VkImageView *imageViews, size_t imageCount,
        ImageData_T **data) {
    impl = new ImageImpl_T;
    for (int i = 0; i < imageCount; i++) {
      data[i] = new ImageData_T;
      data[i]->image = images[i];
      data[i]->view = imageViews[i];
    }
  }

  Image(uint32_t width, uint32_t height, uint32_t depth, Format format,
        VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
        ImageData_T **images, size_t imageCount) {
    impl = new ImageImpl_T;
    impl->width = width;
    impl->height = height;
    impl->depth = depth;
    impl->format = (VkFormat)format;
    impl->imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    CreateImages(usage, properties, images, impl, imageCount);
  }

  Image(uint32_t width, uint32_t height, uint32_t depth, Format format,
        VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
        ImageData_T **images, StagingData_T **staging, size_t imageCount) {
    impl = new ImageImpl_T;
    impl->width = width;
    impl->height = height;
    impl->depth = depth;
    impl->format = (VkFormat)format;
    impl->imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    CreateImages(usage, properties, images, impl, imageCount);
    CreateStagingBuffer(staging, impl, imageCount);
  }

  void transitionImageLayout(VkCommandBuffer commandBuffer, VkImage image,
                             VkFormat format, VkImageLayout oldLayout,
                             VkImageLayout newLayout, uint32_t mipLevels) {
    // Typical Vulkan image layout transition implementation
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = 0;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
        newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
      barrier.srcAccessMask = 0;
      barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
      destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
               newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
      barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
      sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
      destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
               newLayout == VK_IMAGE_LAYOUT_GENERAL) {
      barrier.srcAccessMask = 0;
      barrier.dstAccessMask =
          VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
      sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
      destinationStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT |
                         VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;
    } else {
      // Handle other transitions as needed
      barrier.srcAccessMask = 0;
      barrier.dstAccessMask = 0;
      sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
      destinationStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    }

    vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0,
                         nullptr, 0, nullptr, 1, &barrier);
  }

  void transitionImageLayout(VkImage image, VkFormat format,
                             VkImageLayout oldLayout, VkImageLayout newLayout,
                             uint32_t mipLevels) {
    VkCommandBuffer commandBuffer = ResourceManager::beginSingleTimeCommands();
    transitionImageLayout(commandBuffer, image, format, oldLayout, newLayout,
                          mipLevels);
    ResourceManager::endSingleTimeCommands(commandBuffer);
  }

  size_t formatSize(VkFormat format) {
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

  void CreateImages(VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
                    struct ImageData_T **images, struct ImageImpl_T *impl,
                    size_t imageCount) {

    for (int i = 0; i < imageCount; i++) {
      images[i] = new ImageData_T;
      // createImage(ctx->getDevice(), ctx->getPhysicalDevice(), width, height,
      // depth, format, usage, properties, images[i], deviceMemories[i]);
      VkImageCreateInfo imageCreateInfo = {};
      imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
      imageCreateInfo.imageType =
          impl->depth > 1 ? VK_IMAGE_TYPE_3D : VK_IMAGE_TYPE_2D;
      imageCreateInfo.format = impl->format;
      imageCreateInfo.extent.width = impl->width;
      imageCreateInfo.extent.height = impl->height;
      imageCreateInfo.extent.depth = impl->depth;
      imageCreateInfo.mipLevels = 1;
      imageCreateInfo.arrayLayers = 1;
      imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
      imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
      imageCreateInfo.usage = usage;
      imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
      imageCreateInfo.queueFamilyIndexCount = 0;
      imageCreateInfo.pQueueFamilyIndices = nullptr;
      imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

      if (vkCreateImage(vkZero_core->device, &imageCreateInfo, nullptr,
                        &images[i]->image) != VK_SUCCESS) {
        throw std::runtime_error("failed to create raytracing storage image!");
      }

      VkMemoryRequirements memRequirements;
      vkGetImageMemoryRequirements(vkZero_core->device, images[i]->image,
                                   &memRequirements);

      VkMemoryAllocateInfo allocInfo = {};
      allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
      allocInfo.allocationSize = memRequirements.size;

      VkPhysicalDeviceMemoryProperties memProperties;
      vkGetPhysicalDeviceMemoryProperties(vkZero_core->physicalDevice,
                                          &memProperties);
      for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((memRequirements.memoryTypeBits & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & properties)) {
          allocInfo.memoryTypeIndex = i;
          break;
        }
      }

      if (vkAllocateMemory(vkZero_core->device, &allocInfo, nullptr,
                           &images[i]->memory) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate memory for image!");
      }

      vkBindImageMemory(vkZero_core->device, images[i]->image,
                        images[i]->memory, 0);

      // createImageView(depth, ctx->getDevice(), format, images[i],
      // imageViews[i]);
      VkImageViewCreateInfo viewCreateInfo = {};
      viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
      viewCreateInfo.image = images[i]->image;
      viewCreateInfo.viewType =
          impl->depth > 1 ? VK_IMAGE_VIEW_TYPE_3D : VK_IMAGE_VIEW_TYPE_2D;
      viewCreateInfo.format = (VkFormat)impl->format;
      viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      viewCreateInfo.subresourceRange.baseMipLevel = 0;
      viewCreateInfo.subresourceRange.levelCount = 1;
      viewCreateInfo.subresourceRange.baseArrayLayer = 0;
      viewCreateInfo.subresourceRange.layerCount = 1;

      if (vkCreateImageView(vkZero_core->device, &viewCreateInfo, nullptr,
                            &images[i]->view) != VK_SUCCESS) {
        throw std::runtime_error("failed to creaet raytracing image view!");
      }

      transitionImageLayout(images[i]->image, (VkFormat)impl->format,
                            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL,
                            1);
    }

    VkSamplerCreateInfo samplerCreateInfo = {};
    samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
    samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
    samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_TRANSPARENT_BLACK;
    samplerCreateInfo.anisotropyEnable = VK_FALSE;
    samplerCreateInfo.maxAnisotropy = 1.0f;
    samplerCreateInfo.compareEnable = VK_FALSE;
    samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerCreateInfo.minLod = 0.0f;
    samplerCreateInfo.maxLod = VK_LOD_CLAMP_NONE;
    samplerCreateInfo.mipLodBias = 0.0f;

    if (vkCreateSampler(vkZero_core->device, &samplerCreateInfo, nullptr,
                        &impl->sampler) != VK_SUCCESS) {
      throw std::runtime_error("failed to create sampler");
    }
  }

  void CreateStagingBuffer(struct StagingData_T **staging,
                           struct ImageImpl_T *impl, size_t imageCount) {

    for (int i = 0; i < imageCount; i++) {
      staging[i] = new StagingData_T;
      VkBufferCreateInfo bufferCreateInfo = {};
      bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
      bufferCreateInfo.size =
          impl->width * impl->height * impl->depth * formatSize(impl->format);
      bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
      bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

      if (vkCreateBuffer(vkZero_core->device, &bufferCreateInfo, nullptr,
                         &staging[i]->buffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create voxel staging buffer");
      }

      VkMemoryRequirements stagingMemRequirements;
      vkGetBufferMemoryRequirements(vkZero_core->device, staging[i]->buffer,
                                    &stagingMemRequirements);

      VkMemoryAllocateInfo stagingAllocInfo = {};
      stagingAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
      stagingAllocInfo.allocationSize = stagingMemRequirements.size;
      stagingAllocInfo.memoryTypeIndex = ResourceManager::findMemoryType(
          vkZero_core->physicalDevice, stagingMemRequirements.memoryTypeBits,
          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
              VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

      vkAllocateMemory(vkZero_core->device, &stagingAllocInfo, nullptr,
                       &staging[i]->memory);
      vkBindBufferMemory(vkZero_core->device, staging[i]->buffer,
                         staging[i]->memory, 0);
    }
  }

  void Write(ImageData_T *image, StagingData_T *staging, char *data)
  {
        VkCommandBuffer commandBuffer = ResourceManager::beginSingleTimeCommands();
    Write(commandBuffer, image, staging, data);
    ResourceManager::endSingleTimeCommands(commandBuffer);
  }
  void Write(VkCommandBuffer commandBuffer, ImageData_T *image,
             StagingData_T *staging, char *data)
             {
                    
    
    // TODO adjust the size depending on the format
    void *mappedData;
    vkMapMemory(vkZero_core->device, staging->memory, 0,
                impl->width * impl->height * impl->depth * formatSize(impl->format), 0, &mappedData);
    memcpy(mappedData, data, impl->width * impl->height * impl->depth * formatSize(impl->format));
    vkUnmapMemory(vkZero_core->device, staging->memory);
    VkBufferImageCopy region = {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {impl->width, impl->height, impl->depth};
    vkCmdCopyBufferToImage(commandBuffer, staging->buffer, image->image,
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
             }
  void WriteDescriptor(VkDevice device, VkDescriptorSet descriptorSet,
                       uint32_t binding, uint32_t element,
                       VkDescriptorType type, ImageData_T *image)
                       {
                        VkWriteDescriptorSet descriptorWrite;
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = descriptorSet;
    descriptorWrite.dstBinding = binding;
    descriptorWrite.dstArrayElement = element;
    descriptorWrite.descriptorType = type;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pNext = nullptr;

    VkDescriptorImageInfo imageInfo;
    imageInfo.imageLayout = impl->imageLayout;
    imageInfo.imageView = image->view;
    imageInfo.sampler = impl->sampler;
    
    descriptorWrite.pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
                       }
  void changeLayout(ImageData_T *image, VkImageLayout newLayout, int mipLevels)
  {
        VkCommandBuffer commandBuffer = ResourceManager::beginSingleTimeCommands();
    changeLayout(commandBuffer, image, newLayout, mipLevels);
  ResourceManager::endSingleTimeCommands(commandBuffer);
  }
  void changeLayout(VkCommandBuffer commandBuffer, ImageData_T *image,
                    VkImageLayout newLayout, int mipLevels)
                    {
                            transitionImageLayout(commandBuffer, image->image, impl->format, impl->imageLayout, newLayout, mipLevels);
                    }

  ImageImpl_T *impl;
};

template <int MaxImageCount> class ImageBase : public BindResource, Image {
public:
  ImageBase(uint32_t width, uint32_t height, uint32_t depth, Format format,
            VkImageUsageFlags usage, VkMemoryPropertyFlags properties)
      : Image(width, height, depth, format, usage, properties, images,
              MaxImageCount) {}

  ImageBase(VkImage *vk_images, VkImageView *imageViews, size_t imageCount)
      : Image(vk_images, imageViews, imageCount, images) {}

  void writeDescriptor(VkDevice device, VkDescriptorSet descriptorSet,
                       uint32_t binding, uint32_t element,
                       VkDescriptorType type, int frame) override {
    frame = std::min(frame, maxImages - 1);
    Image::WriteDescriptor(device, descriptorSet, binding, element, type,
                           images[frame]);
  }

  void changeLayout(ImageLayout newLayout, uint32_t index) {
    Image::changeLayout(images[index], newLayout, 1);
  }

  void changeLayout(VkCommandBuffer commandBuffer, ImageLayout newLayout,
                    uint32_t index) {
    Image::changeLayout(commandBuffer, images[index], newLayout, 1);
  }

  ImageData_T *images[MaxImageCount];

  static constexpr int maxImages = MaxImageCount;
};

template <int MaxImageCount>
class StagedImageBase : public BindResource, Image {
public:
  StagedImageBase(uint32_t width, uint32_t height, uint32_t depth,
                  Format format, VkImageUsageFlags usage,
                  VkMemoryPropertyFlags properties)
      : Image(width, height, depth, format, usage, properties, images, staging,
              MaxImageCount) {}

  void changeLayout(ImageLayout newLayout, uint32_t index) {
    Image::changeLayout(images[index], (VkImageLayout)newLayout, 1);
  }

  void changeLayout(VkCommandBuffer commandBuffer, ImageLayout newLayout,
                    uint32_t index) {
    Image::changeLayout(commandBuffer, images[index], (VkImageLayout)newLayout,
                        1);
  }

  void writeDescriptor(VkDevice device, VkDescriptorSet descriptorSet,
                       uint32_t binding, uint32_t element,
                       VkDescriptorType type, int frame) override {
    frame = std::min(frame, maxImages - 1);
    Image::WriteDescriptor(device, descriptorSet, binding, element, type,
                           images[frame]);
  }

  void write(char *data, uint32_t index) {
    Image::Write(images[index], staging[index], data);
  }

  void write(VkCommandBuffer commandBuffer, char *data, uint32_t index) {
    Image::Write(commandBuffer, images[index], staging[index], data);
  }

  ImageData_T *images[MaxImageCount];
  StagingData_T *staging[MaxImageCount];

  static constexpr int maxImages = MaxImageCount;
};

using SwapImage = class ImageBase<2>;
using SharedImage = class ImageBase<1>;
using StagedSwapImage = class StagedImageBase<2>;
using StagedSharedImage = class StagedImageBase<1>;
using AttachmentImage = class ImageBase<3>;
using StagedAttachmentImage = class StagedImageBase<3>;
} // namespace VkZero
