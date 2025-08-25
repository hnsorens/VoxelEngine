#include "image.hpp"
#include "ResourceManager.hpp"
#include "Engine.hpp"

void ImageImpl::CreateImages(uint32_t width, uint32_t height,
        VkFormat format, VkImageTiling tiling,
        VkImageUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkImageLayout initialLayout, size_t imageCount, VkImage* images, VkImageView* imageViews, VkDeviceMemory* deviceMemories, VkSampler* sampler)
{
    std::unique_ptr<VulkanContext>& ctx = VoxelEngine::vulkanContext;
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        ResourceManager::createImage(ctx->getDevice(), ctx->getPhysicalDevice(), width, height, format, tiling, usage, properties, images[i], deviceMemories[i]);
        ResourceManager::createImageView(ctx->getDevice(), format, images[i], imageViews[i]);
        ResourceManager::transitionImageLayout(VoxelEngine::commandManager, ctx, images[i], format, VK_IMAGE_LAYOUT_UNDEFINED, initialLayout, 1);
    }

    VkSamplerCreateInfo samplerCreateInfo = {};
    samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
    samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
    samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_TRANSPARENT_BLACK;
    samplerCreateInfo.anisotropyEnable = VK_FALSE;
    samplerCreateInfo.maxAnisotropy = 1.0f;
    samplerCreateInfo.compareEnable = VK_FALSE;
    samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerCreateInfo.minLod = 0.0f;
    samplerCreateInfo.maxLod = VK_LOD_CLAMP_NONE;
    samplerCreateInfo.mipLodBias = 0.0f;
  
    if (vkCreateSampler(ctx->getDevice(), &samplerCreateInfo, nullptr,
                        sampler) != VK_SUCCESS) {
      throw std::runtime_error("failed to create sampler");
    }
}