#include "ImageHelper.h"
#include "VulkanContext.hpp"
#include "image.hpp"


VkImageView getImageView(Image* image, int index)
{
    return image->imageViews[index];
}

size_t getImageFramebufferCount(Image* image)
{
    return MAX_FRAMES_IN_FLIGHT;
}