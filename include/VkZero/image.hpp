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

class Image {
public:
  Image(VkImage *vk_images, VkImageView *imageViews, size_t imageCount) {
    images.resize(imageCount);
    for (int i = 0; i < imageCount; i++) {
      images[i] = new ImageData_T;
      images[i]->image = vk_images[i];
      images[i]->view = imageViews[i];
    }
  }

  Image(uint32_t width, uint32_t height, uint32_t depth, Format format,
        VkImageUsageFlags usage, VkMemoryPropertyFlags properties, size_t imageCount, bool staging) : width(width), height(height), depth(depth) {
    images.resize(imageCount);
    imageFormat = (VkFormat)format;
    imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    CreateImages(usage, properties, images.data(), imageCount);
    if (staging) {
      stageBuffers.resize(imageCount);
      CreateStagingBuffer(stageBuffers.data(), imageCount);
    }
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
                    struct ImageData_T **images,
                    size_t imageCount) {

    for (int i = 0; i < imageCount; i++) {
      images[i] = new ImageData_T;
      // createImage(ctx->getDevice(), ctx->getPhysicalDevice(), width, height,
      // depth, format, usage, properties, images[i], deviceMemories[i]);
      VkImageCreateInfo imageCreateInfo = {};
      imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
      imageCreateInfo.imageType =
          depth > 1 ? VK_IMAGE_TYPE_3D : VK_IMAGE_TYPE_2D;
      imageCreateInfo.format = imageFormat;
      imageCreateInfo.extent.width = width;
      imageCreateInfo.extent.height = height;
      imageCreateInfo.extent.depth = depth;
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
          depth > 1 ? VK_IMAGE_VIEW_TYPE_3D : VK_IMAGE_VIEW_TYPE_2D;
      viewCreateInfo.format = (VkFormat)imageFormat;
      viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      viewCreateInfo.subresourceRange.baseMipLevel = 0;
      viewCreateInfo.subresourceRange.levelCount = 1;
      viewCreateInfo.subresourceRange.baseArrayLayer = 0;
      viewCreateInfo.subresourceRange.layerCount = 1;

      if (vkCreateImageView(vkZero_core->device, &viewCreateInfo, nullptr,
                            &images[i]->view) != VK_SUCCESS) {
        throw std::runtime_error("failed to creaet raytracing image view!");
      }

      transitionImageLayout(images[i]->image, imageFormat,
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
                        &sampler) != VK_SUCCESS) {
      throw std::runtime_error("failed to create sampler");
    }
  }

  void CreateStagingBuffer(struct StagingData_T **staging, size_t imageCount) {

    for (int i = 0; i < imageCount; i++) {
      staging[i] = new StagingData_T;
      VkBufferCreateInfo bufferCreateInfo = {};
      bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
      bufferCreateInfo.size =
          width * height * depth * formatSize(imageFormat);
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

  void Write(uint32_t index, char *data)
  {
        VkCommandBuffer commandBuffer = ResourceManager::beginSingleTimeCommands();
    Write(commandBuffer, index, data);
    ResourceManager::endSingleTimeCommands(commandBuffer);
  }
  void Write(VkCommandBuffer commandBuffer, uint32_t index, char *data)
             {
    
    // TODO adjust the size depending on the format
    void *mappedData;
    vkMapMemory(vkZero_core->device, stageBuffers[index]->memory, 0,
                width * height * depth * formatSize(imageFormat), 0, &mappedData);
    memcpy(mappedData, data, width * height * depth * formatSize(imageFormat));
    vkUnmapMemory(vkZero_core->device, stageBuffers[index]->memory);
    VkBufferImageCopy region = {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width, height, depth};
    vkCmdCopyBufferToImage(commandBuffer, stageBuffers[index]->buffer, images[index]->image,
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
    imageInfo.imageLayout = imageLayout;
    imageInfo.imageView = image->view;
    imageInfo.sampler = sampler;
    
    descriptorWrite.pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
                       }
  void changeLayout(size_t index, VkImageLayout newLayout, int mipLevels)
  {
        VkCommandBuffer commandBuffer = ResourceManager::beginSingleTimeCommands();
    changeLayout(commandBuffer, index, newLayout, mipLevels);
  ResourceManager::endSingleTimeCommands(commandBuffer);
  }
  void changeLayout(VkCommandBuffer commandBuffer, size_t index,
                    VkImageLayout newLayout, int mipLevels)
                    {
                            transitionImageLayout(commandBuffer, images[index]->image, imageFormat, imageLayout, newLayout, mipLevels);
                    }

  std::vector<ImageData_T*> images;
  std::vector<StagingData_T*> stageBuffers;
  uint32_t width, height, depth;
  VkSampler sampler;
  VkImageLayout imageLayout;
  VkFormat imageFormat;
};

struct MultiImageImpl_T : public BindResource, Image
{
  MultiImageImpl_T(uint32_t width, uint32_t height, uint32_t depth, Format format, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, uint32_t imageCount) : Image(width, height, depth, format, usage, properties, imageCount, false)
  {

  }

  MultiImageImpl_T(VkImage* vk_images, VkImageView* imageViews, size_t imageCount) : Image(vk_images, imageViews, imageCount)
  {

  }

    void changeLayout(ImageLayout newLayout, uint32_t index) {
    Image::changeLayout(index, (VkImageLayout)newLayout, 1);
  }

  void changeLayout(VkCommandBuffer commandBuffer, ImageLayout newLayout,
                    uint32_t index) {
    Image::changeLayout(commandBuffer, index, (VkImageLayout)newLayout, 1);
  }

    void writeDescriptor(VkDevice device, VkDescriptorSet descriptorSet,
                       uint32_t binding, uint32_t element,
                       VkDescriptorType type, int frame) override {
    frame = std::min(frame, (int)images.size() - 1);
    Image::WriteDescriptor(device, descriptorSet, binding, element, type,
                           images[frame]);
  }
};

struct MultiImageBase
{
  MultiImageBase(uint32_t width, uint32_t height, uint32_t depth, Format format, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, uint32_t imageCount)
  {
    impl = new MultiImageImpl_T(width, height, depth, format, usage, properties, imageCount);
  }

  MultiImageBase(VkImage* vk_images, VkImageView* imageViews, size_t imageCount)
  {
    impl = new MultiImageImpl_T(vk_images, imageViews, imageCount);
  }

  void changeLayout(ImageLayout newLayout, uint32_t index) {
    impl->changeLayout(newLayout, index);
  }

  void changeLayout(VkCommandBuffer commandBuffer, ImageLayout newLayout,
                    uint32_t index) {
    impl->changeLayout(commandBuffer, newLayout, index);
  }

  struct MultiImageImpl_T* impl;
};

template <int MaxImageCount> class MultiImage : public MultiImageBase {
public:
  MultiImage(uint32_t width, uint32_t height, uint32_t depth, Format format,
            VkImageUsageFlags usage, VkMemoryPropertyFlags properties)
      : MultiImageBase(width, height, depth, format, usage, properties,
              MaxImageCount) {}

  MultiImage(VkImage *vk_images, VkImageView *imageViews, size_t imageCount)
      : MultiImageBase(vk_images, imageViews, imageCount) {}

  static constexpr int maxImages = MaxImageCount;
};

struct MultiStagedImageImpl_T : public BindResource, Image
{
  MultiStagedImageImpl_T(uint32_t width, uint32_t height, uint32_t depth, Format format, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, uint32_t imageCount): Image(width, height, depth, format, usage, properties, imageCount, true)
  {

  }

    void writeDescriptor(VkDevice device, VkDescriptorSet descriptorSet,
                       uint32_t binding, uint32_t element,
                       VkDescriptorType type, int frame) override {
    frame = std::min(frame, (int)images.size() - 1);
    Image::WriteDescriptor(device, descriptorSet, binding, element, type,
                           images[frame]);
  }

      void changeLayout(ImageLayout newLayout, uint32_t index) {
    Image::changeLayout(index, (VkImageLayout)newLayout, 1);
  }

  void changeLayout(VkCommandBuffer commandBuffer, ImageLayout newLayout,
                    uint32_t index) {
    Image::changeLayout(commandBuffer, index, (VkImageLayout)newLayout, 1);
  }

  void write(char *data, uint32_t index) {
    Image::Write(index, data);
  }

  void write(VkCommandBuffer commandBuffer, char *data, uint32_t index) {
    Image::Write(commandBuffer, index, data);
  }


};

struct MultiStagedImageBase
{
  MultiStagedImageBase(uint32_t width, uint32_t height, uint32_t depth, Format format, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, uint32_t imageCount)
  {
    impl = new MultiStagedImageImpl_T(width, height, depth, format, usage, properties, imageCount);
  }

  void changeLayout(ImageLayout newLayout, uint32_t index) {
    impl->changeLayout(newLayout, index);
  }

  void changeLayout(VkCommandBuffer commandBuffer, ImageLayout newLayout,
                    uint32_t index) {
    impl->changeLayout(commandBuffer, newLayout, index);
  }

    void write(char *data, uint32_t index) {
    impl->write(data, index);
  }

  void write(VkCommandBuffer commandBuffer, char *data, uint32_t index) {
    impl->write(commandBuffer, data, index);
  }

  struct MultiStagedImageImpl_T* impl;
};

template <int MaxImageCount>
class MultiStagedImage : public MultiStagedImageBase {
public:
  MultiStagedImage(uint32_t width, uint32_t height, uint32_t depth,
                  Format format, VkImageUsageFlags usage,
                  VkMemoryPropertyFlags properties) : MultiStagedImageBase(width, height, depth, format, usage, properties, MaxImageCount)
                  {

                  }
  
  static constexpr int maxImages = MaxImageCount;
};

using SwapImage = class MultiImage<2>;
using SharedImage = class MultiImage<1>;
using StagedSwapImage = class MultiStagedImage<2>;
using StagedSharedImage = class MultiStagedImage<1>;
using AttachmentImage = class MultiImage<3>;
using StagedAttachmentImage = class MultiStagedImage<3>;
} // namespace VkZero
