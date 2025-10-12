#include "VkZero/image.hpp"
#include "VkZero/Internal/image_internal.hpp"

using namespace VkZero;

MultiImageBase::MultiImageBase(uint32_t width, uint32_t height, uint32_t depth, Format format, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, uint32_t imageCount)
{
  impl = new MultiImageImpl_T(width, height, depth, format, usage, properties, imageCount);
}

MultiImageBase::MultiImageBase(struct MultiImageImpl_T *impl) : impl(impl) {}

void MultiImageBase::changeLayout(ImageLayout newLayout, uint32_t index) {
  impl->changeLayout(newLayout, index);
}

void MultiImageBase::changeLayout(void* commandBuffer, ImageLayout newLayout,
                  uint32_t index) {
  impl->changeLayout((VkCommandBuffer)commandBuffer, newLayout, index);
}

MultiStagedImageBase::MultiStagedImageBase(uint32_t width, uint32_t height, uint32_t depth, Format format, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, uint32_t imageCount)
{
  impl = new MultiStagedImageImpl_T(width, height, depth, format, usage, properties, imageCount);
}

void MultiStagedImageBase::changeLayout(ImageLayout newLayout, uint32_t index) {
  impl->changeLayout(newLayout, index);
}

void MultiStagedImageBase::changeLayout(void* commandBuffer, ImageLayout newLayout,
                  uint32_t index) {
  impl->changeLayout((VkCommandBuffer)commandBuffer, newLayout, index);
}

void MultiStagedImageBase::write(char *data, uint32_t index) {
  impl->write(data, index);
}

void MultiStagedImageBase::write(void* commandBuffer, char *data, uint32_t index) {
  impl->write((VkCommandBuffer)commandBuffer, data, index);
}

MultiImageImpl_T::MultiImageImpl_T(uint32_t width, uint32_t height, uint32_t depth, Format format, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, uint32_t imageCount) : Image(width, height, depth, format, usage, properties, imageCount, false)
{

}

MultiImageImpl_T::MultiImageImpl_T(VkImage* vk_images, VkImageView* imageViews, size_t imageCount) : Image(vk_images, imageViews, imageCount)
{

}

void MultiImageImpl_T::changeLayout(ImageLayout newLayout, uint32_t index) {
  Image::changeLayout(index, (VkImageLayout)newLayout, 1);
}

void MultiImageImpl_T::changeLayout(VkCommandBuffer commandBuffer, ImageLayout newLayout,
                  uint32_t index) {
  Image::changeLayout(commandBuffer, index, (VkImageLayout)newLayout, 1);
}

void MultiImageImpl_T::writeDescriptor(VkDevice device, VkDescriptorSet descriptorSet,
                      uint32_t binding, uint32_t element,
                      VkDescriptorType type, int frame) {
  frame = std::min(frame, (int)images.size() - 1);
  Image::WriteDescriptor(device, descriptorSet, binding, element, type,
                          images[frame]);
}

  MultiStagedImageImpl_T::MultiStagedImageImpl_T(uint32_t width, uint32_t height, uint32_t depth, Format format, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, uint32_t imageCount): Image(width, height, depth, format, usage, properties, imageCount, true)
  {

  }

    void MultiStagedImageImpl_T::writeDescriptor(VkDevice device, VkDescriptorSet descriptorSet,
                       uint32_t binding, uint32_t element,
                       VkDescriptorType type, int frame) {
    frame = std::min(frame, (int)images.size() - 1);
    Image::WriteDescriptor(device, descriptorSet, binding, element, type,
                           images[frame]);
  }

      void MultiStagedImageImpl_T::changeLayout(ImageLayout newLayout, uint32_t index) {
    Image::changeLayout(index, (VkImageLayout)newLayout, 1);
  }

  void MultiStagedImageImpl_T::changeLayout(VkCommandBuffer commandBuffer, ImageLayout newLayout,
                    uint32_t index) {
    Image::changeLayout(commandBuffer, index, (VkImageLayout)newLayout, 1);
  }

  void MultiStagedImageImpl_T::write(char *data, uint32_t index) {
    Image::Write(index, data);
  }

  void MultiStagedImageImpl_T::write(VkCommandBuffer commandBuffer, char *data, uint32_t index) {
    Image::Write(commandBuffer, index, data);
  }
