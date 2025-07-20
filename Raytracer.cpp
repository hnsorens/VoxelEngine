#include "Raytracer.hpp"
#include "Camera.hpp"
#include "CommandManager.hpp"
#include "PipelineManager.hpp"
#include "VoxelWorld.hpp"
#include "VulkanContext.hpp"
#include <memory>
#include <stdexcept>
#include <vulkan/vulkan_core.h>
#include "ResourceManager.hpp"

Raytracer::Raytracer(std::unique_ptr<CommandManager>& commandManager, std::unique_ptr<VulkanContext>& vulkanContext, std::unique_ptr<VoxelWorld>& voxelWorld, std::unique_ptr<Camera>& camera) 

{
    createRaytracingResources(commandManager, vulkanContext);
    createRaytracingPipeline(vulkanContext->getDevice(), camera->uniformBuffer, voxelWorld->voxelImageView, voxelWorld->voxelTextureSampler, voxelWorld->voxelChunkMapImageView);
}

Raytracer::~Raytracer() {}


const VkPipeline& Raytracer::getPipeline() const { return raytracingPipeline; }
const VkPipelineLayout& Raytracer::getPipelineLayout() const { return raytracingPipelineLayout; }
const VkDescriptorSet& Raytracer::getDescriptorSet(int i) const { return raytracingDescriptorSets[i]; }
const VkImageView& Raytracer::getStorageImage(int i) const { return raytracingStorageImageView[i]; }

void Raytracer::createRaytracingResources(std::unique_ptr<CommandManager>& commandManager, std::unique_ptr<VulkanContext>& vulkanContext) 
{
    raytracingStorageImage.resize(MAX_FRAMES_IN_FLIGHT);
    raytracingStorageImageView.resize(MAX_FRAMES_IN_FLIGHT);
    raytracingStorageMemory.resize(MAX_FRAMES_IN_FLIGHT);

    raytracingPositionStorageImage.resize(MAX_FRAMES_IN_FLIGHT);
    raytracingPositionStorageImageView.resize(MAX_FRAMES_IN_FLIGHT);
    raytracingPositionStorageMemory.resize(MAX_FRAMES_IN_FLIGHT);

    raytracingLightStorageImageX.resize(MAX_FRAMES_IN_FLIGHT);
    raytracingLightStorageImageViewX.resize(MAX_FRAMES_IN_FLIGHT);
    raytracingLightStorageMemoryX.resize(MAX_FRAMES_IN_FLIGHT);

    raytracingLightStorageImageY.resize(MAX_FRAMES_IN_FLIGHT);
    raytracingLightStorageImageViewY.resize(MAX_FRAMES_IN_FLIGHT);
    raytracingLightStorageMemoryY.resize(MAX_FRAMES_IN_FLIGHT);

    raytracingLightStorageImageZ.resize(MAX_FRAMES_IN_FLIGHT);
    raytracingLightStorageImageViewZ.resize(MAX_FRAMES_IN_FLIGHT);
    raytracingLightStorageMemoryZ.resize(MAX_FRAMES_IN_FLIGHT);

    raytracingLightStorageImageW.resize(MAX_FRAMES_IN_FLIGHT);
    raytracingLightStorageImageViewW.resize(MAX_FRAMES_IN_FLIGHT);
    raytracingLightStorageMemoryW.resize(MAX_FRAMES_IN_FLIGHT);

    VkDevice device = vulkanContext->getDevice();
    VkPhysicalDevice physicalDevice = vulkanContext->getPhysicalDevice();
    
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
            ResourceManager::createImage(device, physicalDevice, RAYTRACE_WIDTH, RAYTRACE_HEIGHT, 
                                VK_FORMAT_R16G16B16A16_UNORM, VK_IMAGE_TILING_OPTIMAL, 
                                 VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
                                raytracingStorageImage[i], raytracingStorageMemory[i]);
            ResourceManager::createImageView(device, VK_FORMAT_R16G16B16A16_UNORM, 
                                raytracingStorageImage[i], raytracingStorageImageView[i]);
            ResourceManager::transitionImageLayout(commandManager, vulkanContext, 
                                 raytracingStorageImage[i], VK_FORMAT_R16G16B16A16_UNORM, 
                             VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, 1);
        
            ResourceManager::createImage(device, physicalDevice, RAYTRACE_WIDTH, RAYTRACE_HEIGHT, 
                                VK_FORMAT_R32_UINT, VK_IMAGE_TILING_OPTIMAL, 
                                 VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
                                raytracingPositionStorageImage[i], raytracingPositionStorageMemory[i]);
            ResourceManager::createImageView(device, VK_FORMAT_R32_UINT, 
                                raytracingPositionStorageImage[i], raytracingPositionStorageImageView[i]);
            ResourceManager::transitionImageLayout(commandManager, vulkanContext, 
                                 raytracingPositionStorageImage[i], VK_FORMAT_R32_UINT, 
                             VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, 1);
                         
        
            ResourceManager::createImage(device, physicalDevice, RAYTRACE_WIDTH, RAYTRACE_HEIGHT, 
                                VK_FORMAT_R32_UINT, VK_IMAGE_TILING_OPTIMAL, 
                                 VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
                                raytracingLightStorageImageX[i], raytracingLightStorageMemoryX[i]);
            ResourceManager::createImageView(device, VK_FORMAT_R32_UINT, 
                                raytracingLightStorageImageX[i], raytracingLightStorageImageViewX[i]);
            ResourceManager::transitionImageLayout(commandManager, vulkanContext, 
                                 raytracingLightStorageImageX[i], VK_FORMAT_R32_UINT, 
                             VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, 1);

            ResourceManager::createImage(device, physicalDevice, RAYTRACE_WIDTH, RAYTRACE_HEIGHT, 
                                VK_FORMAT_R32_UINT, VK_IMAGE_TILING_OPTIMAL, 
                                 VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
                                raytracingLightStorageImageY[i], raytracingLightStorageMemoryY[i]);
            ResourceManager::createImageView(device, VK_FORMAT_R32_UINT, 
                                raytracingLightStorageImageY[i], raytracingLightStorageImageViewY[i]);
            ResourceManager::transitionImageLayout(commandManager, vulkanContext, 
                                 raytracingLightStorageImageY[i], VK_FORMAT_R32_UINT, 
                             VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, 1);

            ResourceManager::createImage(device, physicalDevice, RAYTRACE_WIDTH, RAYTRACE_HEIGHT, 
                                VK_FORMAT_R32_UINT, VK_IMAGE_TILING_OPTIMAL, 
                                 VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
                                raytracingLightStorageImageZ[i], raytracingLightStorageMemoryZ[i]);
            ResourceManager::createImageView(device, VK_FORMAT_R32_UINT, 
                                raytracingLightStorageImageZ[i], raytracingLightStorageImageViewZ[i]);
            ResourceManager::transitionImageLayout(commandManager, vulkanContext, 
                                 raytracingLightStorageImageZ[i], VK_FORMAT_R32_UINT, 
                             VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, 1);

            ResourceManager::createImage(device, physicalDevice, RAYTRACE_WIDTH, RAYTRACE_HEIGHT, 
                                VK_FORMAT_R32_UINT, VK_IMAGE_TILING_OPTIMAL, 
                                 VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
                                raytracingLightStorageImageW[i], raytracingLightStorageMemoryW[i]);
            ResourceManager::createImageView(device, VK_FORMAT_R32_UINT, 
                                raytracingLightStorageImageW[i], raytracingLightStorageImageViewW[i]);
            ResourceManager::transitionImageLayout(commandManager, vulkanContext, 
                                 raytracingLightStorageImageW[i], VK_FORMAT_R32_UINT, 
                             VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, 1);
    }
}

void Raytracer::createRaytracingPipeline(VkDevice device, std::vector<VkBuffer>& uniformBuffer, std::vector<VkImageView>& voxelImageView, VkSampler voxelTextureSampler, std::vector<VkImageView>& voxelChunkMapImageView) 
{
    auto raygenShaderCode = ResourceManager::readFile("rgen.spv");
    auto missShaderCode = ResourceManager::readFile("rmiss.spv");

    VkShaderModule raygenShaderModule = PipelineManager::createShaderModule(device, raygenShaderCode);
    VkShaderModule missShaderModule = PipelineManager::createShaderModule(device, missShaderCode);

    VkPipelineShaderStageCreateInfo raygenShaderStageInfo{};
    raygenShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    raygenShaderStageInfo.stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    raygenShaderStageInfo.module = raygenShaderModule;
    raygenShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo missShaderStageInfo{};
    missShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    missShaderStageInfo.stage = VK_SHADER_STAGE_MISS_BIT_KHR;
    missShaderStageInfo.module = missShaderModule;
    missShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {raygenShaderStageInfo, missShaderStageInfo};

    VkRayTracingShaderGroupCreateInfoKHR shaderGroups[2] = {};

    shaderGroups[0].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
    shaderGroups[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
    shaderGroups[0].generalShader = 0;
    shaderGroups[0].closestHitShader = VK_SHADER_UNUSED_KHR;
    shaderGroups[0].anyHitShader = VK_SHADER_UNUSED_KHR;
    shaderGroups[0].intersectionShader = VK_SHADER_UNUSED_KHR;

    shaderGroups[1].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
    shaderGroups[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
    shaderGroups[1].generalShader = 1;
    shaderGroups[1].closestHitShader = VK_SHADER_UNUSED_KHR;
    shaderGroups[1].anyHitShader = VK_SHADER_UNUSED_KHR;
    shaderGroups[1].intersectionShader = VK_SHADER_UNUSED_KHR;


    VkDescriptorSetLayoutBinding storageImageBinding = {};
    storageImageBinding.binding = 0;
    storageImageBinding.descriptorCount = 1;
    storageImageBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    storageImageBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    storageImageBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding uniformBinding = {};
    uniformBinding.binding = 1;
    uniformBinding.descriptorCount = 1;
    uniformBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uniformBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    uniformBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding positionStorageImageBinding = {};
    positionStorageImageBinding.binding = 2;
    positionStorageImageBinding.descriptorCount = 1;
    positionStorageImageBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    positionStorageImageBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    positionStorageImageBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding voxelTexture = {};
    voxelTexture.binding = 3;
    voxelTexture.descriptorCount = 512;
    voxelTexture.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    voxelTexture.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    voxelTexture.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding voxelChunkMapTexture = {};
    voxelChunkMapTexture.binding = 4;
    voxelChunkMapTexture.descriptorCount = 1;
    voxelChunkMapTexture.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    voxelChunkMapTexture.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    voxelChunkMapTexture.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding lightStorageImageBindingX = {};
    lightStorageImageBindingX.binding = 5;
    lightStorageImageBindingX.descriptorCount = 1;
    lightStorageImageBindingX.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    lightStorageImageBindingX.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    lightStorageImageBindingX.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding lightStorageImageBindingY = {};
    lightStorageImageBindingY.binding = 6;
    lightStorageImageBindingY.descriptorCount = 1;
    lightStorageImageBindingY.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    lightStorageImageBindingY.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    lightStorageImageBindingY.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding lightStorageImageBindingZ = {};
    lightStorageImageBindingZ.binding = 7;
    lightStorageImageBindingZ.descriptorCount = 1;
    lightStorageImageBindingZ.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    lightStorageImageBindingZ.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    lightStorageImageBindingZ.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding lightStorageImageBindingW = {};
    lightStorageImageBindingW.binding = 8;
    lightStorageImageBindingW.descriptorCount = 1;
    lightStorageImageBindingW.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    lightStorageImageBindingW.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    lightStorageImageBindingW.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding bindings[] = {storageImageBinding, uniformBinding, voxelTexture, positionStorageImageBinding, voxelChunkMapTexture, lightStorageImageBindingX, lightStorageImageBindingY, lightStorageImageBindingZ, lightStorageImageBindingW};

    VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {};
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutCreateInfo.bindingCount = 9;
    layoutCreateInfo.pBindings = bindings;
    layoutCreateInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;

    VkDescriptorBindingFlags bindless_flags = 
        VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT |
            VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT;
            VkDescriptorBindingFlags flags[] = {0, 0, bindless_flags, 0, 0, 0, 0, 0, 0};

    VkDescriptorSetLayoutBindingFlagsCreateInfoEXT extended_info{
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT,
        nullptr};
    extended_info.bindingCount = 9;
    extended_info.pBindingFlags = flags;

    layoutCreateInfo.pNext = &extended_info;

    if (vkCreateDescriptorSetLayout(device, &layoutCreateInfo, nullptr, &raytracingDescriptorSetLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create raytracing descriptor set layout!");
    }

            VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    pushConstantRange.offset = 0;
    pushConstantRange.size = 8;

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = 1;
    pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;
    pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
    pipelineLayoutCreateInfo.pSetLayouts = &raytracingDescriptorSetLayout;

    if (vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &raytracingPipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create raytracing pipeline layout!");
    }

    printf("PIPELINE LAYOUT %i\n", 1);

    VkRayTracingPipelineCreateInfoKHR pipelineCreateInfo{};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
    pipelineCreateInfo.stageCount = 2;
    pipelineCreateInfo.pStages = shaderStages;
    pipelineCreateInfo.groupCount = 2;
    pipelineCreateInfo.pGroups = shaderGroups;
    pipelineCreateInfo.maxPipelineRayRecursionDepth = 1;
    pipelineCreateInfo.layout = raytracingPipelineLayout;

    PFN_vkCreateRayTracingPipelinesKHR vkCreateRayTracingPipelinesKHR = reinterpret_cast<PFN_vkCreateRayTracingPipelinesKHR>(vkGetDeviceProcAddr(device, "vkCreateRayTracingPipelinesKHR"));
    if (vkCreateRayTracingPipelinesKHR(device, VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &raytracingPipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create raytracing pipeline!");
    }
    printf("PIPELINE LAYOUT %i\n", 1);
    vkDestroyShaderModule(device, raygenShaderModule, nullptr);
    vkDestroyShaderModule(device, missShaderModule, nullptr);


    // Pool

    VkDescriptorPoolSize storageImagePoolSize = {};
    storageImagePoolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    storageImagePoolSize.descriptorCount = 2;

    VkDescriptorPoolSize uboPoolSize = {};
    uboPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboPoolSize.descriptorCount = 2;

    VkDescriptorPoolSize upperVoxelPoolSize = {};
    upperVoxelPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    upperVoxelPoolSize.descriptorCount = 600;

    VkDescriptorPoolSize poolSizes[] = {storageImagePoolSize, uboPoolSize, upperVoxelPoolSize};

    VkDescriptorPoolCreateInfo poolCreateInfo = {};
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolCreateInfo.poolSizeCount = 3;
    poolCreateInfo.pPoolSizes = poolSizes;
    poolCreateInfo.maxSets = 50;

    if (vkCreateDescriptorPool(device, &poolCreateInfo, nullptr, &raytracingDescriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create raytracing descriptor pool!");
    }

    // Updating Descriptor Set

    raytracingDescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);

    VkDescriptorSetLayout setLayouts[] = {raytracingDescriptorSetLayout, raytracingDescriptorSetLayout};

    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = raytracingDescriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = setLayouts;
    
    if (vkAllocateDescriptorSets(device, &allocInfo, raytracingDescriptorSets.data()))
    {
        throw std::runtime_error("Failed to create raytracing descriptor set!");
    }

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        VkDescriptorImageInfo positionImageInfo = {};
        positionImageInfo.imageView = raytracingPositionStorageImageView[i];
        positionImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

        VkWriteDescriptorSet writePositionStorageDescriptorSet = {};
        writePositionStorageDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writePositionStorageDescriptorSet.dstSet = raytracingDescriptorSets[i];
        writePositionStorageDescriptorSet.dstBinding = 2;
        writePositionStorageDescriptorSet.dstArrayElement = 0;
        writePositionStorageDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        writePositionStorageDescriptorSet.descriptorCount = 1;
        writePositionStorageDescriptorSet.pImageInfo = &positionImageInfo;

        VkDescriptorImageInfo lightImageInfoX = {};
        lightImageInfoX.imageView = raytracingLightStorageImageViewX[i];
        lightImageInfoX.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

        VkDescriptorImageInfo lightImageInfoY = {};
        lightImageInfoY.imageView = raytracingLightStorageImageViewY[i];
        lightImageInfoY.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

        VkDescriptorImageInfo lightImageInfoZ = {};
        lightImageInfoZ.imageView = raytracingLightStorageImageViewZ[i];
        lightImageInfoZ.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

        VkDescriptorImageInfo lightImageInfoW = {};
        lightImageInfoW.imageView = raytracingLightStorageImageViewW[i];
        lightImageInfoW.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

        VkWriteDescriptorSet writeLightStorageDescriptorSetX = {};
        writeLightStorageDescriptorSetX.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeLightStorageDescriptorSetX.dstSet = raytracingDescriptorSets[i];
        writeLightStorageDescriptorSetX.dstBinding = 5;
        writeLightStorageDescriptorSetX.dstArrayElement = 0;
        writeLightStorageDescriptorSetX.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        writeLightStorageDescriptorSetX.descriptorCount = 1;
        writeLightStorageDescriptorSetX.pImageInfo = &lightImageInfoX;

        VkWriteDescriptorSet writeLightStorageDescriptorSetY = {};
        writeLightStorageDescriptorSetY.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeLightStorageDescriptorSetY.dstSet = raytracingDescriptorSets[i];
        writeLightStorageDescriptorSetY.dstBinding = 6;
        writeLightStorageDescriptorSetY.dstArrayElement = 0;
        writeLightStorageDescriptorSetY.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        writeLightStorageDescriptorSetY.descriptorCount = 1;
        writeLightStorageDescriptorSetY.pImageInfo = &lightImageInfoY;

        VkWriteDescriptorSet writeLightStorageDescriptorSetZ = {};
        writeLightStorageDescriptorSetZ.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeLightStorageDescriptorSetZ.dstSet = raytracingDescriptorSets[i];
        writeLightStorageDescriptorSetZ.dstBinding = 7;
        writeLightStorageDescriptorSetZ.dstArrayElement = 0;
        writeLightStorageDescriptorSetZ.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        writeLightStorageDescriptorSetZ.descriptorCount = 1;
        writeLightStorageDescriptorSetZ.pImageInfo = &lightImageInfoZ;

        VkWriteDescriptorSet writeLightStorageDescriptorSetW = {};
        writeLightStorageDescriptorSetW.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeLightStorageDescriptorSetW.dstSet = raytracingDescriptorSets[i];
        writeLightStorageDescriptorSetW.dstBinding = 8;
        writeLightStorageDescriptorSetW.dstArrayElement = 0;
        writeLightStorageDescriptorSetW.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        writeLightStorageDescriptorSetW.descriptorCount = 1;
        writeLightStorageDescriptorSetW.pImageInfo = &lightImageInfoW;

        VkDescriptorImageInfo imageInfo = {};
        imageInfo.imageView = raytracingStorageImageView[i];
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

        VkWriteDescriptorSet writeStorageDescriptorSet = {};
        writeStorageDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeStorageDescriptorSet.dstSet = raytracingDescriptorSets[i];
        writeStorageDescriptorSet.dstBinding = 0;
        writeStorageDescriptorSet.dstArrayElement = 0;
        writeStorageDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        writeStorageDescriptorSet.descriptorCount = 1;
        writeStorageDescriptorSet.pImageInfo = &imageInfo;

        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffer[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(TransformUBO);

        VkWriteDescriptorSet writeTransformDescriptorSet = {};
        writeTransformDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeTransformDescriptorSet.dstSet = raytracingDescriptorSets[i];
        writeTransformDescriptorSet.dstBinding = 1;
        writeTransformDescriptorSet.dstArrayElement = 0;
        writeTransformDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writeTransformDescriptorSet.descriptorCount = 1;
        writeTransformDescriptorSet.pBufferInfo = &bufferInfo;

        std::vector<VkDescriptorImageInfo> voxelImageInfos(512);

        for (int i = 0; i < 512; i++)
        {
            voxelImageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            voxelImageInfos[i].imageView = voxelImageView[i];
            voxelImageInfos[i].sampler = voxelTextureSampler;
        }
        

        VkWriteDescriptorSet voxelDescriptorSet = {};
        voxelDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        voxelDescriptorSet.dstSet = raytracingDescriptorSets[i];
        voxelDescriptorSet.dstBinding = 3;
        voxelDescriptorSet.dstArrayElement = 0;
        voxelDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        voxelDescriptorSet.descriptorCount = 512;
        voxelDescriptorSet.pImageInfo = voxelImageInfos.data();


        VkDescriptorImageInfo voxelChunkMapImageInfo = {};
        voxelChunkMapImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        voxelChunkMapImageInfo.imageView = voxelChunkMapImageView[i];
        voxelChunkMapImageInfo.sampler = voxelTextureSampler;

        VkWriteDescriptorSet voxelChunkMapDescriptorSet = {};
        voxelChunkMapDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        voxelChunkMapDescriptorSet.dstSet = raytracingDescriptorSets[i];
        voxelChunkMapDescriptorSet.dstBinding = 4;
        voxelChunkMapDescriptorSet.dstArrayElement = 0;
        voxelChunkMapDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        voxelChunkMapDescriptorSet.descriptorCount = 1;
        voxelChunkMapDescriptorSet.pImageInfo = &voxelChunkMapImageInfo;

        VkWriteDescriptorSet writeDescriptorSets[] = {writeStorageDescriptorSet, writeTransformDescriptorSet, voxelDescriptorSet, writeLightStorageDescriptorSetX, writeLightStorageDescriptorSetY, writeLightStorageDescriptorSetZ, writeLightStorageDescriptorSetW, writePositionStorageDescriptorSet, voxelChunkMapDescriptorSet};

        vkUpdateDescriptorSets(device, 9, writeDescriptorSets, 0, nullptr);
    }
}


void Raytracer::recordRaytracingCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, uint8_t section) {} 