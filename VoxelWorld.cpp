#include "VoxelWorld.hpp"
#include "CommandManager.hpp"
#include "ResourceManager.hpp"
#include "VulkanContext.hpp"
#include <algorithm>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

VoxelWorld::VoxelWorld(std::unique_ptr<VulkanContext>& vulkanContext, std::unique_ptr<CommandManager>& commandManager) 
{
    chunkPosition = glm::ivec3(0, 0, 0);
    noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);

    voxelTexture.resize(512);
    voxelImageView.resize(512);
    voxelTexturesMemory.resize(512);
    voxelStagingBuffer.resize(512);
    voxelStagingBufferMemory.resize(512);
    for (int i = 0; i < 512; i++)
    {
        VkImageCreateInfo imageInfo = {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_3D;
        imageInfo.extent.width = 128;
        imageInfo.extent.height = 128;
        imageInfo.extent.depth = 128;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = VK_FORMAT_R8_UINT;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

        if (vkCreateImage(vulkanContext->getDevice(), &imageInfo, nullptr, &voxelTexture[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture image!");
        }

        // Allocate memory for the image
        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(vulkanContext->getDevice(), voxelTexture[i], &memRequirements);

        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = ResourceManager::findMemoryType(vulkanContext->getPhysicalDevice(), memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        if (vkAllocateMemory(vulkanContext->getDevice(), &allocInfo, nullptr, &voxelTexturesMemory[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate memory for texture image!");
        }

        vkBindImageMemory(vulkanContext->getDevice(), voxelTexture[i], voxelTexturesMemory[i], 0);

        VkImageViewCreateInfo viewInfo = {};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = voxelTexture[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
        viewInfo.format = VK_FORMAT_R8_UINT;
        viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(vulkanContext->getDevice(), &viewInfo, nullptr, &voxelImageView[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture image view!");
        }
        ResourceManager::transitionImageLayout(commandManager, vulkanContext, voxelTexture[i], VK_FORMAT_R8_UINT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL, 1);

        VkBufferCreateInfo bufferCreateInfo = {};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = 128*128*128;
    bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(vulkanContext->getDevice(), &bufferCreateInfo, nullptr, &voxelStagingBuffer[i]) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create voxel staging buffer");
    }

    VkMemoryRequirements stagingMemRequirements;
    vkGetBufferMemoryRequirements(vulkanContext->getDevice(), voxelStagingBuffer[i], &stagingMemRequirements);

    VkMemoryAllocateInfo stagingAllocInfo = {};
    stagingAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    stagingAllocInfo.allocationSize = stagingMemRequirements.size;
    stagingAllocInfo.memoryTypeIndex = ResourceManager::findMemoryType(vulkanContext->getPhysicalDevice(), stagingMemRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    vkAllocateMemory(vulkanContext->getDevice(), &stagingAllocInfo, nullptr, &voxelStagingBufferMemory[i]);
    vkBindBufferMemory(vulkanContext->getDevice(), voxelStagingBuffer[i], voxelStagingBufferMemory[i], 0);
    }

    VkSamplerCreateInfo samplerCreateInfo = {};
    samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCreateInfo.magFilter = VK_FILTER_NEAREST;
    samplerCreateInfo.minFilter = VK_FILTER_NEAREST;
    samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
    samplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;

    vkCreateSampler(vulkanContext->getDevice(), &samplerCreateInfo, nullptr, &voxelTextureSampler);

    startWorkers(10);
    for (int i = 0; i < 512; i++)
    {
        requestChunk(i, 1.0f);
    }
    sortChunks();
    std::this_thread::sleep_for(std::chrono::seconds(10));



        // Staging buffer to update the texture
    voxelChunkMapStagingBuffer.resize(MAX_FRAMES_IN_FLIGHT);
    voxelChunkMapStagingBufferMemory.resize(MAX_FRAMES_IN_FLIGHT);
    voxelChunkMapTexture.resize(MAX_FRAMES_IN_FLIGHT);
    voxelChunkMapTexturesMemory.resize(MAX_FRAMES_IN_FLIGHT);
    voxelChunkMapImageView.resize(MAX_FRAMES_IN_FLIGHT);

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
            // Create the image (texture)
        VkImageCreateInfo voxelChunkMapImageInfo = {};
        voxelChunkMapImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        voxelChunkMapImageInfo.imageType = VK_IMAGE_TYPE_3D;
        voxelChunkMapImageInfo.extent.width = 8;
        voxelChunkMapImageInfo.extent.height = 8;
        voxelChunkMapImageInfo.extent.depth = 8;
        voxelChunkMapImageInfo.mipLevels = 1;
        voxelChunkMapImageInfo.arrayLayers = 1;
        voxelChunkMapImageInfo.format = VK_FORMAT_R16_UINT;
        voxelChunkMapImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        voxelChunkMapImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        voxelChunkMapImageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
        voxelChunkMapImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        voxelChunkMapImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

        if (vkCreateImage(vulkanContext->getDevice(), &voxelChunkMapImageInfo, nullptr, &voxelChunkMapTexture[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture image!");
        }

        // Allocate memory for the image
        VkMemoryRequirements voxelChunkMapMemRequirements;
        vkGetImageMemoryRequirements(vulkanContext->getDevice(), voxelChunkMapTexture[i], &voxelChunkMapMemRequirements);

        VkMemoryAllocateInfo voxelChunkMapAllocInfo = {};
        voxelChunkMapAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        voxelChunkMapAllocInfo.allocationSize = voxelChunkMapMemRequirements.size;
        voxelChunkMapAllocInfo.memoryTypeIndex = ResourceManager::findMemoryType(vulkanContext->getPhysicalDevice(), voxelChunkMapMemRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        if (vkAllocateMemory(vulkanContext->getDevice(), &voxelChunkMapAllocInfo, nullptr, &voxelChunkMapTexturesMemory[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate memory for texture image!");
        }

        vkBindImageMemory(vulkanContext->getDevice(), voxelChunkMapTexture[i], voxelChunkMapTexturesMemory[i], 0);

        VkImageViewCreateInfo voxelChunkMapViewInfo = {};
        voxelChunkMapViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        voxelChunkMapViewInfo.image = voxelChunkMapTexture[i];
        voxelChunkMapViewInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
        voxelChunkMapViewInfo.format = VK_FORMAT_R16_UINT;
        voxelChunkMapViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        voxelChunkMapViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        voxelChunkMapViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        voxelChunkMapViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        voxelChunkMapViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        voxelChunkMapViewInfo.subresourceRange.baseMipLevel = 0;
        voxelChunkMapViewInfo.subresourceRange.levelCount = 1;
        voxelChunkMapViewInfo.subresourceRange.baseArrayLayer = 0;
        voxelChunkMapViewInfo.subresourceRange.layerCount = 1;
        voxelChunkMapViewInfo.pNext = nullptr;

        if (vkCreateImageView(vulkanContext->getDevice(), &voxelChunkMapViewInfo, nullptr, &voxelChunkMapImageView[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture image view!");
        }



        VkBufferCreateInfo voxelChunkMapBufferCreateInfo = {};
        voxelChunkMapBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        voxelChunkMapBufferCreateInfo.size = 8*8*8*2;
        voxelChunkMapBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        voxelChunkMapBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(vulkanContext->getDevice(), &voxelChunkMapBufferCreateInfo, nullptr, &voxelChunkMapStagingBuffer[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create voxel staging buffer");
        }

        VkMemoryRequirements voxelChunkMapStagingMemRequirements;
        vkGetBufferMemoryRequirements(vulkanContext->getDevice(), voxelChunkMapStagingBuffer[i], &voxelChunkMapStagingMemRequirements);

        VkMemoryAllocateInfo voxelChunkMapStagingAllocInfo = {};
        voxelChunkMapStagingAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        voxelChunkMapStagingAllocInfo.allocationSize = voxelChunkMapStagingMemRequirements.size;
        voxelChunkMapStagingAllocInfo.memoryTypeIndex = ResourceManager::findMemoryType(vulkanContext->getPhysicalDevice(), voxelChunkMapStagingMemRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        vkAllocateMemory(vulkanContext->getDevice(), &voxelChunkMapStagingAllocInfo, nullptr, &voxelChunkMapStagingBufferMemory[i]);
        vkBindBufferMemory(vulkanContext->getDevice(), voxelChunkMapStagingBuffer[i], voxelChunkMapStagingBufferMemory[i], 0);
        
        ResourceManager::transitionImageLayout(commandManager, vulkanContext, voxelChunkMapTexture[i], VK_FORMAT_R16_UINT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL, 1);
    }
    // Voxel Data

    voxelChunkMapData = new uint16_t[8*8*8]();

    for (int i = 0; i < 8*8*8; i++)
    {
        voxelChunkMapData[i] = i;
    }
}
VoxelWorld::~VoxelWorld() {}

bool VoxelWorld::chunkSort(uint16_t a, uint16_t b)
{
    glm::vec3 cameraPosition{-512,-512,-512};
    glm::vec3 cameraGlobal = (glm::vec3(chunkPosition) * 128.0f) - cameraPosition;
    return glm::distance(cameraGlobal, glm::vec3(voxelData[a].position)) > glm::distance(cameraGlobal, glm::vec3(voxelData[b].position));
}

void VoxelWorld::generateTerrain() 
{
        auto it = std::chrono::system_clock::now();
        #define vox(x,y,z) voxelData[(int)std::floor((float)(z) / 128.0f) * 64 + (int)std::floor((float)(y) / 128.0f) * 8 + (int)std::floor((float)(x) / 128.0f)].data[((z) % 128) * 128 * 128 + ((y) % 128) * 128 + ((x) % 128)]
        #define voxChunk(chunk, x,y,z) chunk[((z) % 128) * 128 * 128 + ((y) % 128) * 128 + ((x) % 128)]


        #define TERRAIN_SCALE 2
        #define TERRAIN_AREA (1024)

        //printf("hwc: %i", std::thread::hardware_concurrency());
        printf("Generating\n");
        const int WORKERS = 20;
        const int SLICE_THICKNESS = TERRAIN_AREA / WORKERS;

        std::thread* threads[WORKERS]{};

        for (int i = 0; i < WORKERS; i++)
        {
            // threads[i] = new std::thread{&TerrainWork, voxelData, &noise, i * SLICE_THICKNESS, SLICE_THICKNESS};
        }

        puts("");
        for (int i = 0; i < WORKERS; i++)
        {
            threads[i]->join();
            printf("\rProgress: %.1f", (float)i / WORKERS * 100);
        }

        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - it);
}
void VoxelWorld::startWorkers(int numThreads) 
{
    voxelData.resize(512);
    chunkQueue.resize(513);
    chunkQueue[0] = 0;
    chunkUpdateQueue.resize(513);
    chunkUpdateQueue[0] = 0;

   
    for (int x = 0; x < 8; x++)
    {
        for (int y = 0; y < 8; y++)
        {
            for (int z = 0; z < 8; z++)
            {
                size_t chunkIndex = z*64 + y*8 + x;
                voxelData[chunkIndex] = {};
                voxelData[chunkIndex].data = new uint8_t[128 * 128 * 128]();
                voxelData[chunkIndex].position = glm::ivec3(x * 128, y * 128, z * 128);
            } 
        }
    }

    for (int i = 0; i < numThreads; i++) {
        workers.emplace_back([this]{chunkWorker();});
    }
}
void VoxelWorld::stopWorkers() 
{
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        stopThreads = true;
    }
    queueCond.notify_all();
    for (auto& worker : workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}
void VoxelWorld::requestChunk(uint16_t chunkID, float priority) 
{
    // std::fill(voxelData[chunkID].data, voxelData[chunkID].data + 128 * 128 * 128, 0);
    std::lock_guard<std::mutex> lock(queueMutex);

    // chunkGenerationID[chunkID]++; // Increment generation ID
    if (voxelData[chunkID].inQueue) return;
    chunkQueue[++chunkQueue[0]] = chunkID;
    voxelData[chunkID].inQueue = true;
    queueCond.notify_one();
}
void VoxelWorld::sortChunks()
{
    std::lock_guard<std::mutex> lock(queueMutex);
    if (chunkQueue[0] > 5)
    std::sort(chunkQueue.begin() + 1, chunkQueue.begin() + chunkQueue[0], [this](uint16_t a, uint16_t b){return chunkSort(a,b);});
}
void VoxelWorld::updateVoxelChunkMap(int modValue, int offset) 
{
    switch (offset)
    {
    case 1:
        chunkPosition += glm::ivec3(1, 0, 0);
        break;
    case 7:
        chunkPosition -= glm::ivec3(1, 0, 0);
        break;
    case 8:
        chunkPosition += glm::ivec3(0, 1, 0);
        break;
    case 56:
        chunkPosition += glm::ivec3(0, -1, 0);
        break;  
    case 64:
        chunkPosition += glm::ivec3(0, 0, 1);
        break;
    case 448:
        chunkPosition += glm::ivec3(0, 0, -1);
        break;  
    
    default:
        break;
    }

    const int chunkSize = 8 * 128; // 1024
    for (int z = 0; z < 8; z++) {
        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                int index = z * 8 * 8 + y * 8 + x;
                int chunk = voxelChunkMapData[index];
                int chunkAxis = chunk % modValue;

                uint16_t chunkID = (chunk - chunkAxis) + (chunkAxis + offset) % modValue;
                switch (modValue) {
                    case 8:
                        if (offset == 1 && x == 7)
                        {
                            voxelData[chunkID].position.x += chunkSize;
                            requestChunk(chunkID, 0.0f);
                        }
                        else if (offset == 7 && x == 0)
                        {
                            voxelData[chunkID].position.x -= chunkSize;
                            requestChunk(chunkID, 0.0f);
                        }
                        break;
                    case 64:
                        if (offset == 8 && y == 7)
                        {
                            voxelData[chunkID].position.y += chunkSize;
                            requestChunk(chunkID, 0.0f);
                        }
                        else if (offset == 56 && y == 0)
                        {
                            voxelData[chunkID].position.y -= chunkSize;
                            requestChunk(chunkID, 0.0f);
                        }
                        break;
                    case 512:
                        if (offset == 64 && z == 7)
                        {
                            voxelData[chunkID].position.z += chunkSize;
                            requestChunk(chunkID, 0.0f);
                        }
                        else if (offset == 448 && z == 0)
                        {
                            voxelData[chunkID].position.z -= chunkSize;
                            requestChunk(chunkID, 0.0f);
                        }
                        break;
                }

                voxelChunkMapData[index] = chunkID;
            }
        }
    }
}
void VoxelWorld::updateVoxels(VkCommandBuffer commandBuffer, std::unique_ptr<VulkanContext>& vulkanContext, int currentImage) 
{
    ResourceManager::transitionImageLayout(commandBuffer, voxelChunkMapTexture[currentImage], VK_FORMAT_R16_UINT, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1);
    for (int i = 0; i < 16; i++)
    {
        if (chunkUpdateQueue[0] == 0)
        {
            break;
        }
        
        uint16_t ID = chunkUpdateQueue[chunkUpdateQueue[0]--];
        ResourceManager::transitionImageLayout(commandBuffer, voxelTexture[ID], VK_FORMAT_R8_UINT, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1);

        void* mappedData;
        vkMapMemory(vulkanContext->getDevice(), voxelStagingBufferMemory[ID], 0, 128*128*128, 0, &mappedData);
        memcpy(mappedData, (char*)voxelData[ID].data, 128*128*128);
        vkUnmapMemory(vulkanContext->getDevice(), voxelStagingBufferMemory[ID]);
        VkBufferImageCopy region = {};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = { 128, 128, 128 };
        vkCmdCopyBufferToImage(commandBuffer, voxelStagingBuffer[ID], voxelTexture[ID], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
        ResourceManager::transitionImageLayout(commandBuffer, voxelTexture[ID], VK_FORMAT_R8_UINT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL, 1);
    }

    void* mappedVoxelChunkMapData;
    vkMapMemory(vulkanContext->getDevice(), voxelChunkMapStagingBufferMemory[currentImage], 0, 8*8*8*2, 0, &mappedVoxelChunkMapData);
    memcpy(mappedVoxelChunkMapData, voxelChunkMapData, 8*8*8*2);
    vkUnmapMemory(vulkanContext->getDevice(), voxelChunkMapStagingBufferMemory[currentImage]);

    VkBufferImageCopy voxelChunkMapRegion = {};
    voxelChunkMapRegion.bufferOffset = 0;
    voxelChunkMapRegion.bufferRowLength = 0;
    voxelChunkMapRegion.bufferImageHeight = 0;
    voxelChunkMapRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    voxelChunkMapRegion.imageSubresource.mipLevel = 0;
    voxelChunkMapRegion.imageSubresource.baseArrayLayer = 0;
    voxelChunkMapRegion.imageSubresource.layerCount = 1;
    voxelChunkMapRegion.imageOffset = { 0,0,0 };
    voxelChunkMapRegion.imageExtent = { 8,8,8 };

    vkCmdCopyBufferToImage(commandBuffer, voxelChunkMapStagingBuffer[currentImage], voxelChunkMapTexture[currentImage], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &voxelChunkMapRegion);
    ResourceManager::transitionImageLayout(commandBuffer, voxelChunkMapTexture[currentImage], VK_FORMAT_R16_UINT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL, 1);
}
void VoxelWorld::chunkWorker() 
{
    while (!stopThreads) {
        uint16_t chunkID;
        uint32_t genID;

        {
            std::unique_lock<std::mutex> lock(queueMutex);
            queueCond.wait(lock, [&] { return chunkQueue[0] != 0 || stopThreads; });

            if (stopThreads) return;

            if (chunkQueue[0] > 0) {
                chunkID = chunkQueue[chunkQueue[0]--];
            } else {
                continue;  // Recheck condition
            }
        }
        voxelData[chunkID].inQueue = false;

        generateChunk(voxelData[chunkID]);
        chunkUpdateQueue[++chunkUpdateQueue[0]] = chunkID;
    }
}
void VoxelWorld::generateChunk(VoxelChunk& chunk) 
{
    glm::ivec3 position = chunk.position;
        uint8_t* chunkData = chunk.data;
        #pragma omp parallel for collapse(2)
        for (int z = 0; z < 128; z++)
        {
            for (int y = 0; y < 128; y++)
            {
                for (int x = 0; x < 128; x++)
                {
                    glm::ivec3 samplePosition = position + glm::ivec3(x, y, z);
                    uint8_t base = 0;//0b11100000;
                    float v = (noise.GetNoise(samplePosition.x * 0.1f * TERRAIN_SCALE, samplePosition.y * 0.1f * TERRAIN_SCALE, samplePosition.z * 0.4f * TERRAIN_SCALE) + noise.GetNoise(samplePosition.x * 1.5f * TERRAIN_SCALE, samplePosition.y * 1.5f * TERRAIN_SCALE, samplePosition.z * 1.0f * TERRAIN_SCALE) * 0.03) / 1.03f;
                    float v_above = (noise.GetNoise(samplePosition.x * 0.1f * TERRAIN_SCALE, samplePosition.y * 0.1f * TERRAIN_SCALE, (samplePosition.z-1) * 0.4f * TERRAIN_SCALE) + noise.GetNoise(samplePosition.x * 1.5f * TERRAIN_SCALE, samplePosition.y * 1.5f * TERRAIN_SCALE, (samplePosition.z-1) * 1.0f * TERRAIN_SCALE) * 0.03) / 1.03f;
                    int material = v <= 0.1;
                    if (material != MAT_AIR)
                    {
                        if ((v_above <= 0.1))
                        {
                            float v = noise.GetNoise(x * 10.0f * TERRAIN_SCALE, y * 10.0f * TERRAIN_SCALE, z * 10.0f * TERRAIN_SCALE) * 0.7f + noise.GetNoise(x * 5.0f * TERRAIN_SCALE, y * 5.0f * TERRAIN_SCALE, z * 5.0f * TERRAIN_SCALE) * 0.3f;
                            if (v > 0.33)
                            {
                                material = MAT_STONE;
                            }
                            else if (v > -0.33)
                            {
                                material = MAT_STONE2;
                            }
                            else
                            {
                                material = MAT_STONE3;
                            }
                        }
                      
                    }

                    voxChunk(chunkData, x, y, z) = base | material;
                }
            }
        }

        // Set the base
        #pragma omp parallel for collapse(2)
        for (int x = 0; x < 128; x+=2)
        {
            for (int y = 0; y < 128; y+=2)
            {
                for (int z = 0; z < 128; z+=2)
                {
                    if (voxel_mat(voxChunk(chunkData, x, y, z)) == MAT_AIR && 
                        voxel_mat(voxChunk(chunkData, x+1, y, z)) == MAT_AIR && 
                        voxel_mat(voxChunk(chunkData, x, y+1, z)) == MAT_AIR && 
                        voxel_mat(voxChunk(chunkData, x, y, z+1)) == MAT_AIR && 
                        voxel_mat(voxChunk(chunkData, x+1, y, z+1)) == MAT_AIR && 
                        voxel_mat(voxChunk(chunkData, x+1, y+1, z)) == MAT_AIR && 
                        voxel_mat(voxChunk(chunkData, x, y+1, z+1)) == MAT_AIR && 
                        voxel_mat(voxChunk(chunkData, x+1, y+1, z+1)) == MAT_AIR)
                    {
                        uint8_t value = 1 << 5;
                        voxChunk(chunkData, x, y, z) = value;
                        voxChunk(chunkData, x+1, y, z) = value;
                        voxChunk(chunkData, x, y+1, z) = value;
                        voxChunk(chunkData, x, y, z+1) = value;
                        voxChunk(chunkData, x+1, y, z+1) = value;
                        voxChunk(chunkData, x+1, y+1, z) = value;
                        voxChunk(chunkData, x, y+1, z+1) = value;
                        voxChunk(chunkData, x+1, y+1, z+1) = value;
                    }
                }
            }
        }
        #pragma omp parallel for collapse(2)
        for (int x = 0; x < 128; x+=4)
        {
            for (int y = 0; y < 128; y+=4)
            {
                for (int z = 0; z < 128; z+=4)
                {
                    if (voxel_base(voxChunk(chunkData, x, y, z)) == 1 && 
                        voxel_base(voxChunk(chunkData, x+2, y, z)) == 1 && 
                        voxel_base(voxChunk(chunkData, x, y+2, z)) == 1 && 
                        voxel_base(voxChunk(chunkData, x, y, z+2)) == 1 && 
                        voxel_base(voxChunk(chunkData, x+2, y, z+2)) == 1 && 
                        voxel_base(voxChunk(chunkData, x+2, y+2, z)) == 1 && 
                        voxel_base(voxChunk(chunkData, x, y+2, z+2)) == 1 && 
                        voxel_base(voxChunk(chunkData, x+2, y+2, z+2)) == 1)
                    {
                        uint8_t value = 2 << 5;
                        for (int x2 = 0; x2 < 4; x2++)
                        {
                            for (int y2 = 0; y2 < 4; y2++)
                            {
                                for (int z2 = 0; z2 < 4; z2++)
                                {
                                    voxChunk(chunkData, x+x2, y+y2, z+z2) = value;
                                }
                            }
                        }
                    }
                }
            }
        }

        #pragma omp parallel for collapse(2)
        for (int x = 0; x < 128; x+=8)
        {
            for (int y = 0; y < 128; y+=8)
            {
                for (int z = 0; z < 128; z+=8)
                {
                    if (voxel_base(voxChunk(chunkData, x, y, z)) == 2 && 
                        voxel_base(voxChunk(chunkData, x+4, y, z)) == 2 && 
                        voxel_base(voxChunk(chunkData, x, y+4, z)) == 2 && 
                        voxel_base(voxChunk(chunkData, x, y, z+4)) == 2 && 
                        voxel_base(voxChunk(chunkData, x+4, y, z+4)) == 2 && 
                        voxel_base(voxChunk(chunkData, x+4, y+4, z)) == 2 && 
                        voxel_base(voxChunk(chunkData, x, y+4, z+4)) == 2 && 
                        voxel_base(voxChunk(chunkData, x+4, y+4, z+4)) == 2)
                    {
                        uint8_t value = 3 << 5;
                        for (int x2 = 0; x2 < 8; x2++)
                        {
                            for (int y2 = 0; y2 < 8; y2++)
                            {
                                for (int z2 = 0; z2 < 8; z2++)
                                {
                                    voxChunk(chunkData, x+x2, y+y2, z+z2) = value;
                                }
                            }
                        }
                    }
                }
            }
        }
        #pragma omp parallel for collapse(2)
        for (int x = 0; x < 128; x+=16)
        {
            for (int y = 0; y < 128; y+=16)
            {
                for (int z = 0; z < 128; z+=16)
                {
                    if (voxel_base(voxChunk(chunkData, x, y, z)) == 3 && 
                        voxel_base(voxChunk(chunkData, x+8, y, z)) == 3 && 
                        voxel_base(voxChunk(chunkData, x, y+8, z)) == 3 && 
                        voxel_base(voxChunk(chunkData, x, y, z+8)) == 3 && 
                        voxel_base(voxChunk(chunkData, x+8, y, z+8)) == 3 && 
                        voxel_base(voxChunk(chunkData, x+8, y+8, z)) == 3 && 
                        voxel_base(voxChunk(chunkData, x, y+8, z+8)) == 3 && 
                        voxel_base(voxChunk(chunkData, x+8, y+8, z+8)) == 3)
                    {
                        uint8_t value = 4 << 5;
                        for (int x2 = 0; x2 < 16; x2++)
                        {
                            for (int y2 = 0; y2 < 16; y2++)
                            {
                                for (int z2 = 0; z2 < 16; z2++)
                                {
                                    voxChunk(chunkData, x+x2, y+y2, z+z2) = value;
                                }
                            }
                        }
                    }
                }
            }
        }
        #pragma omp parallel for collapse(2)
        for (int x = 0; x < 128; x+=32)
        {
            for (int y = 0; y < 128; y+=32)
            {
                for (int z = 0; z < 128; z+=32)
                {
                    if (voxel_base(voxChunk(chunkData, x, y, z)) == 4 && 
                        voxel_base(voxChunk(chunkData, x+16, y, z)) == 4 && 
                        voxel_base(voxChunk(chunkData, x, y+16, z)) == 4 && 
                        voxel_base(voxChunk(chunkData, x, y, z+16)) == 4 && 
                        voxel_base(voxChunk(chunkData, x+16, y, z+16)) == 4 && 
                        voxel_base(voxChunk(chunkData, x+16, y+16, z)) == 4 && 
                        voxel_base(voxChunk(chunkData, x, y+16, z+16)) == 4 && 
                        voxel_base(voxChunk(chunkData, x+16, y+16, z+16)) == 4)
                    {
                        uint8_t value = 5 << 5;
                        for (int x2 = 0; x2 < 32; x2++)
                        {
                            for (int y2 = 0; y2 < 32; y2++)
                            {
                                for (int z2 = 0; z2 < 32; z2++)
                                {
                                    voxChunk(chunkData, x+x2, y+y2, z+z2) = value;
                                }
                            }
                        }
                    }
                }
            }
        }
}
uint8_t* VoxelWorld::getChunk(glm::vec3 position) 
{ 
    return voxelData[voxelChunkMapData[((int)position.z / 128) * 64 + ((int)position.y / 128) * 8 + ((int)position.x / 128)]].data;
}

uint16_t VoxelWorld::getChunkID(glm::vec3 position)
{
    return voxelChunkMapData[((int)position.z / 128) * 64 + ((int)position.y / 128) * 8 + ((int)position.x / 128)];
}