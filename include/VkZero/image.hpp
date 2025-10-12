#pragma once

#include "VkZero/Internal/core_internal.hpp"
#include "VkZero/Internal/bind_resource_internal.hpp"
#include "VkZero/info.hpp"
#include "VkZero/Internal/resource_manager_internal.hpp"
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

struct MultiImageBase {
  MultiImageBase(uint32_t width, uint32_t height, uint32_t depth, Format format,
                 VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
                 uint32_t imageCount);
  MultiImageBase(VkImage *vk_images, VkImageView *imageViews,
                 size_t imageCount);
  void changeLayout(ImageLayout newLayout, uint32_t index);
  void changeLayout(VkCommandBuffer commandBuffer, ImageLayout newLayout,
                    uint32_t index);
  struct MultiImageImpl_T *impl;
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

struct MultiStagedImageBase {
  MultiStagedImageBase(uint32_t width, uint32_t height, uint32_t depth,
                       Format format, VkImageUsageFlags usage,
                       VkMemoryPropertyFlags properties, uint32_t imageCount);
  void changeLayout(ImageLayout newLayout, uint32_t index);
  void changeLayout(VkCommandBuffer commandBuffer, ImageLayout newLayout,
                    uint32_t index);
  void write(char *data, uint32_t index);
  void write(VkCommandBuffer commandBuffer, char *data, uint32_t index);

  struct MultiStagedImageImpl_T *impl;
};

template <int MaxImageCount>
class MultiStagedImage : public MultiStagedImageBase {
public:
  MultiStagedImage(uint32_t width, uint32_t height, uint32_t depth,
                   Format format, VkImageUsageFlags usage,
                   VkMemoryPropertyFlags properties)
      : MultiStagedImageBase(width, height, depth, format, usage, properties,
                             MaxImageCount) {}

  static constexpr int maxImages = MaxImageCount;
};

using SwapImage = class MultiImage<2>;
using SharedImage = class MultiImage<1>;
using StagedSwapImage = class MultiStagedImage<2>;
using StagedSharedImage = class MultiStagedImage<1>;
using AttachmentImage = class MultiImage<3>;
using StagedAttachmentImage = class MultiStagedImage<3>;
} // namespace VkZero
