#pragma once

#include <vulkan/vulkan_core.h>

VkImageView getImageView(class AttachmentImage* image, int index);
size_t getImageFramebufferCount(class AttachmentImage* image);