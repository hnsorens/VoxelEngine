#pragma once

#include "CommandManager.hpp"
#include "ResourceManager.hpp"
#include "VulkanContext.hpp"
#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class Image
{
    Image(std::unique_ptr<VulkanContext>& ctx,
        std::unique_ptr<CommandManager>& commandManager,
            uint32_t width, uint32_t height,
            VkFormat format, VkImageTiling tiling,
            VkImageUsageFlags usage,
            VkMemoryPropertyFlags properties,
            VkImage &image, VkDeviceMemory &imageMemory, VkImageLayout initialLayout)
    {
        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            ResourceManager::createImage(ctx->getDevice(), ctx->getPhysicalDevice(), width, height, format, tiling, usage, properties, images[i], deviceMemories[i]);
            ResourceManager::createImageView(ctx->getDevice(), format, images[i], imageViews[i]);
            ResourceManager::transitionImageLayout(commandManager, ctx, images[i], format, VK_IMAGE_LAYOUT_UNDEFINED, initialLayout, 1);
        }
    }

    VkImage images[MAX_FRAMES_IN_FLIGHT];
    VkImageView imageViews[MAX_FRAMES_IN_FLIGHT];
    VkDeviceMemory deviceMemories[MAX_FRAMES_IN_FLIGHT];
};