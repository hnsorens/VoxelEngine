#include "image.hpp"
#include "ResourceManager.hpp"
#include "Engine.hpp"

Image::Image(uint32_t width, uint32_t height,
        VkFormat format, VkImageTiling tiling,
        VkImageUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkImageLayout initialLayout) : imageLayout(initialLayout)
{
    std::unique_ptr<VulkanContext>& ctx = VoxelEngine::vulkanContext;
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        ResourceManager::createImage(ctx->getDevice(), ctx->getPhysicalDevice(), width, height, format, tiling, usage, properties, images[i], deviceMemories[i]);
        ResourceManager::createImageView(ctx->getDevice(), format, images[i], imageViews[i]);
        ResourceManager::transitionImageLayout(VoxelEngine::commandManager, ctx, images[i], format, VK_IMAGE_LAYOUT_UNDEFINED, initialLayout, 1);
    }
}