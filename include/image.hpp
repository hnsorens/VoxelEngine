#pragma once

#include <vector>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

struct Image
{
    VkImage image;
    VkImageView imageView;
    VkDeviceMemory deviceMemory;
};

class ImageBuilder
{
public:
    ImageBuilder();

    Image build();

private:
};