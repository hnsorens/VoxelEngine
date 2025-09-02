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
        // ResourceManager::createImage(ctx->getDevice(), ctx->getPhysicalDevice(), width, height, depth, format, tiling, usage, properties, images[i], deviceMemories[i]);
        VkImageCreateInfo imageCreateInfo = {};
        imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageCreateInfo.imageType = depth > 1 ? VK_IMAGE_TYPE_3D : VK_IMAGE_TYPE_2D;
        imageCreateInfo.format = format;
        imageCreateInfo.extent.width = width;
        imageCreateInfo.extent.height = height;
        imageCreateInfo.extent.depth = depth;
        imageCreateInfo.mipLevels = 1;
        imageCreateInfo.arrayLayers = 1;
        imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageCreateInfo.tiling = tiling;
        imageCreateInfo.usage = usage;
        imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageCreateInfo.queueFamilyIndexCount = 0;
        imageCreateInfo.pQueueFamilyIndices = nullptr;
        imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        if (vkCreateImage(ctx->getDevice(), &imageCreateInfo, nullptr, &images[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create raytracing storage image!");
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(ctx->getDevice(), images[i], &memRequirements);

        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;

        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(ctx->getPhysicalDevice(), &memProperties);
        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((memRequirements.memoryTypeBits & (1 << i)) &&
                (memProperties.memoryTypes[i].propertyFlags & properties)) {
            allocInfo.memoryTypeIndex = i;
            break;
            }
        }

        if (vkAllocateMemory(ctx->getDevice(), &allocInfo, nullptr, &deviceMemories[i]) !=
            VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate memory for image!");
        }

        vkBindImageMemory(ctx->getDevice(), images[i], deviceMemories[i], 0);

        // ResourceManager::createImageView(depth, ctx->getDevice(), format, images[i], imageViews[i]);
        VkImageViewCreateInfo viewCreateInfo = {};
        viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewCreateInfo.image = images[i];
        viewCreateInfo.viewType = depth > 1 ? VK_IMAGE_VIEW_TYPE_3D : VK_IMAGE_VIEW_TYPE_2D;
        viewCreateInfo.format = format;
        viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewCreateInfo.subresourceRange.baseMipLevel = 0;
        viewCreateInfo.subresourceRange.levelCount = 1;
        viewCreateInfo.subresourceRange.baseArrayLayer = 0;
        viewCreateInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(ctx->getDevice(), &viewCreateInfo, nullptr, &imageViews[i]) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to creaet raytracing image view!");
        }

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

void ImageImpl::CreateStagingBuffer(uint32_t width, uint32_t height, uint32_t depth, VkFormat format, VkBuffer* voxelStagingBuffer, VkDeviceMemory* voxelStagingBufferMemory, size_t imageCount)
{
    std::unique_ptr<VulkanContext>& ctx = VoxelEngine::vulkanContext;

    for (int i = 0; i < imageCount; i++)
    {
        VkBufferCreateInfo bufferCreateInfo = {};
        bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferCreateInfo.size = width * height * depth * formatSize(format);
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

void ImageImpl::Write(uint32_t imageIndex, uint32_t width, uint32_t height, uint32_t depth, VkFormat format, VkDeviceMemory stagingBufferMemory, VkBuffer stagingBuffer, VkImage image, char* data)
{
    std::unique_ptr<VulkanContext>& ctx = VoxelEngine::vulkanContext;
    std::unique_ptr<CommandManager>& commandManager = VoxelEngine::commandManager;

    VkCommandBuffer commandBuffer = commandManager->beginSingleTimeCommands(ctx);
    Write(commandBuffer, imageIndex, width, height, depth, format, stagingBufferMemory, stagingBuffer, image, data);
    commandManager->endSingleTimeCommands(ctx, commandBuffer);
}

void ImageImpl::Write(VkCommandBuffer commandBuffer, uint32_t imageIndex, uint32_t width, uint32_t height, uint32_t depth, VkFormat format, VkDeviceMemory stagingBufferMemory, VkBuffer stagingBuffer, VkImage image, char* data)
{
    std::unique_ptr<VulkanContext>& ctx = VoxelEngine::vulkanContext;
    std::unique_ptr<CommandManager>& commandManager = VoxelEngine::commandManager;
    
    // TODO adjust the size depending on the format
    void *mappedData;
    vkMapMemory(ctx->getDevice(), stagingBufferMemory, 0,
                width * height * depth * formatSize(format), 0, &mappedData);
    memcpy(mappedData, data, width * height * depth * formatSize(format));
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