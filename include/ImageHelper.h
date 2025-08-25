#pragma once

#include <vulkan/vulkan_core.h>

VkImageView getImageView(class Image* image, int index);
size_t getImageFramebufferCount(class Image* image);