#include "image.hpp"
#include "CommandManager.hpp"
#include "ResourceManager.hpp"
#include "Engine.hpp"
#include <cstdint>
#include <vulkan/vulkan_core.h>

void ImageImpl::CreateImages(uint32_t width, uint32_t height, uint32_t depth,
        VkFormat format, VkImageTiling tiling,
        VkImageUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkImageLayout initialLayout, size_t imageCount, VkImage* images, VkImageView* imageViews, VkDeviceMemory* deviceMemories, VkSampler* sampler)
{
    std::unique_ptr<VulkanContext>& ctx = VoxelEngine::vulkanContext;
    for (int i = 0; i < imageCount; i++)
    {
        ResourceManager::createImage(ctx->getDevice(), ctx->getPhysicalDevice(), width, height, depth, format, tiling, usage, properties, images[i], deviceMemories[i]);
        ResourceManager::createImageView(depth, ctx->getDevice(), format, images[i], imageViews[i]);
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

void ImageImpl::CreateStagingBuffer(uint32_t width, uint32_t height, uint32_t depth, VkBuffer* voxelStagingBuffer, VkDeviceMemory* voxelStagingBufferMemory, size_t imageCount)
{
    std::unique_ptr<VulkanContext>& ctx = VoxelEngine::vulkanContext;

    for (int i = 0; i < imageCount; i++)
    {
        VkBufferCreateInfo bufferCreateInfo = {};
        bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferCreateInfo.size = width * height * depth;
        bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(ctx->getDevice(), &bufferCreateInfo, nullptr,
                        &voxelStagingBuffer[i]) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create voxel staging buffer");
        }

        VkMemoryRequirements stagingMemRequirements;
        vkGetBufferMemoryRequirements(ctx->getDevice(),
                                    voxelStagingBuffer[i],
                                    &stagingMemRequirements);

        VkMemoryAllocateInfo stagingAllocInfo = {};
        stagingAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        stagingAllocInfo.allocationSize = stagingMemRequirements.size;
        stagingAllocInfo.memoryTypeIndex = ResourceManager::findMemoryType(
            ctx->getPhysicalDevice(),
            stagingMemRequirements.memoryTypeBits,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        vkAllocateMemory(ctx->getDevice(), &stagingAllocInfo, nullptr,
                        &voxelStagingBufferMemory[i]);
        vkBindBufferMemory(ctx->getDevice(), voxelStagingBuffer[i],
                        voxelStagingBufferMemory[i], 0);
    }
}

void ImageImpl::Write(uint32_t imageIndex, uint32_t width, uint32_t height, uint32_t depth, VkDeviceMemory stagingBufferMemory, VkBuffer stagingBuffer, VkImage image, char* data)
{
    std::unique_ptr<VulkanContext>& ctx = VoxelEngine::vulkanContext;
    std::unique_ptr<CommandManager>& commandManager = VoxelEngine::commandManager;

    VkCommandBuffer commandBuffer = commandManager->beginSingleTimeCommands(ctx);
    Write(commandBuffer, imageIndex, width, height, depth, stagingBufferMemory, stagingBuffer, image, data);
    commandManager->endSingleTimeCommands(ctx, commandBuffer);
}

void ImageImpl::Write(VkCommandBuffer commandBuffer, uint32_t imageIndex, uint32_t width, uint32_t height, uint32_t depth, VkDeviceMemory stagingBufferMemory, VkBuffer stagingBuffer, VkImage image, char* data)
{
    std::unique_ptr<VulkanContext>& ctx = VoxelEngine::vulkanContext;
    std::unique_ptr<CommandManager>& commandManager = VoxelEngine::commandManager;
    
    // TODO adjust the size depending on the format
    void *mappedData;
    vkMapMemory(ctx->getDevice(), stagingBufferMemory, 0,
                width * height * depth, 0, &mappedData);
    memcpy(mappedData, data, width * height * depth);
    vkUnmapMemory(ctx->getDevice(), stagingBufferMemory);
    VkBufferImageCopy region = {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width, height, depth};
    vkCmdCopyBufferToImage(commandBuffer, stagingBuffer, image,
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}

void ImageImpl::changeLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, int mipLevels)
{
    std::unique_ptr<VulkanContext>& ctx = VoxelEngine::vulkanContext;
    std::unique_ptr<CommandManager>& commandManager = VoxelEngine::commandManager;

    VkCommandBuffer commandBuffer = commandManager->beginSingleTimeCommands(ctx);
    changeLayout(commandBuffer, image, format, oldLayout, newLayout, mipLevels);
    commandManager->endSingleTimeCommands(ctx, commandBuffer);
}

void ImageImpl::changeLayout(VkCommandBuffer commandBuffer, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, int mipLevels)
{
    ResourceManager::transitionImageLayout(commandBuffer, image, format, oldLayout, newLayout, mipLevels);
}