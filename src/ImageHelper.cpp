#include "ImageHelper.h"
#include "VulkanContext.hpp"
#include "image.hpp"


VkImageView getImageView(AttachmentImage* image, int index)
{
    return image->imageViews[index];
}

size_t getImageFramebufferCount(AttachmentImage* image)
{
    return MAX_FRAMES_IN_FLIGHT;
}