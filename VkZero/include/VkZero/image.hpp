#pragma once

#include "VkZero/types.hpp"
#include <cstdint>
#include <cstdio>
#include <cstring>

namespace VkZero {

struct MultiImageBase {
  MultiImageBase(uint32_t width, uint32_t height, uint32_t depth, Format format,
                 uint32_t usage, uint32_t properties,
                 uint32_t imageCount);
  MultiImageBase(struct MultiImageImpl_T *impl);
  void changeLayout(ImageLayout newLayout, uint32_t index);
  void changeLayout(void* commandBuffer, ImageLayout newLayout,
                    uint32_t index);
  struct MultiImageImpl_T *impl;
};

template <int MaxImageCount> class MultiImage : public MultiImageBase {
public:
  MultiImage(uint32_t width, uint32_t height, uint32_t depth, Format format,
             uint32_t usage, uint32_t properties)
      : MultiImageBase(width, height, depth, format, usage, properties,
                       MaxImageCount) {}

  MultiImage(struct MultiImageImpl_T *impl)
      : MultiImageBase(impl) {}

  static constexpr int maxImages = MaxImageCount;
};

struct MultiStagedImageBase {
  MultiStagedImageBase(uint32_t width, uint32_t height, uint32_t depth,
                       Format format, uint32_t usage,
                       uint32_t properties, uint32_t imageCount);
  void changeLayout(ImageLayout newLayout, uint32_t index);
  void changeLayout(void* commandBuffer, ImageLayout newLayout,
                    uint32_t index);
  void write(char *data, uint32_t index);
  void write(void* commandBuffer, char *data, uint32_t index);

  struct MultiStagedImageImpl_T *impl;
};

template <int MaxImageCount>
class MultiStagedImage : public MultiStagedImageBase {
public:
  MultiStagedImage(uint32_t width, uint32_t height, uint32_t depth,
                   Format format, uint32_t usage,
                   uint32_t properties)
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
