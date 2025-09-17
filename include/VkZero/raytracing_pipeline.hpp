#pragma once

#include "shader_group.hpp"
#include "shader_resource_set.hpp"
#include "graphics_pipeline.hpp"
#include "VkZero/resource_manager.hpp"
#include <vulkan/vulkan_core.h>
#include <vector>
#include <tuple>
#include <memory>
#include <stdexcept>
#include <cstring>
#include "VkZero/Internal/shader_resource_set_internal.hpp"

#define ALIGN_UP(value, alignment)                                             \
  (((value) + (alignment) - 1) & ~((alignment) - 1))

namespace VkZero
{
    template <typename ShaderGroup, typename... ShaderResourcesBindings>
    class RaytracingPipeline
    {
        static_assert(PipelineDetails::graphics_pipeline_validator<ShaderGroup, ShaderResourcesBindings...>, "Graphics Pipeline Invalid");

    public:

        using Attachments = ShaderGroup::Attachments;

        RaytracingPipeline(ShaderGroup& shaderGroup, ShaderResourcesBindings&... resources) :
        resources(resources...), m_shaderGroup(shaderGroup.impl),
        pipelineLayout([&](){

            std::vector<VkDescriptorSetLayout> descriptorSetLayouts;

            (descriptorSetLayouts.push_back(resources.impl->descriptorSetLayout), ...);

            VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
            pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipelineLayoutInfo.setLayoutCount = descriptorSetLayouts.size();
            pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();

            VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};

            pipelineLayoutInfo.pPushConstantRanges = m_shaderGroup->m_ranges.data();
            pipelineLayoutInfo.pushConstantRangeCount = m_shaderGroup->m_ranges.size();

            VkPipelineLayout layout;
            if (vkCreatePipelineLayout(vkZero_core->device, &pipelineLayoutInfo, nullptr,
                                        &layout) != VK_SUCCESS) {
                throw std::runtime_error("failed to create pipeline layout!");
            }


            return layout;
        }())
        {
            VkRayTracingShaderGroupCreateInfoKHR shaderGroups[2] = {};

            shaderGroups[0].sType =
                VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
            shaderGroups[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
            shaderGroups[0].generalShader = 1;
            shaderGroups[0].closestHitShader = VK_SHADER_UNUSED_KHR;
            shaderGroups[0].anyHitShader = VK_SHADER_UNUSED_KHR;
            shaderGroups[0].intersectionShader = VK_SHADER_UNUSED_KHR;

            shaderGroups[1].sType =
                VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
            shaderGroups[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
            shaderGroups[1].generalShader = 0;
            shaderGroups[1].closestHitShader = VK_SHADER_UNUSED_KHR;
            shaderGroups[1].anyHitShader = VK_SHADER_UNUSED_KHR;
            shaderGroups[1].intersectionShader = VK_SHADER_UNUSED_KHR;
            
            pipelineInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
            // TODO save the shader arrays, so if it goes out of scope the references are still there
            pipelineInfo.stageCount = m_shaderGroup->m_shaders.size();
            pipelineInfo.pStages = m_shaderGroup->m_shaders.data();
            pipelineInfo.groupCount = 2;
            pipelineInfo.pGroups = shaderGroups;
            pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
            pipelineInfo.maxPipelineRayRecursionDepth = 1;
            pipelineInfo.layout = pipelineLayout;

            PFN_vkCreateRayTracingPipelinesKHR vkCreateRayTracingPipelinesKHR = reinterpret_cast<PFN_vkCreateRayTracingPipelinesKHR>(
            vkGetDeviceProcAddr(vkZero_core->device, "vkCreateRayTracingPipelinesKHR"));
            if (vkCreateRayTracingPipelinesKHR(vkZero_core->device, VK_NULL_HANDLE, VK_NULL_HANDLE, 1,
                                                &pipelineInfo, nullptr,
                                                &pipeline) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create raytracing pipeline!");
            }

            VkPhysicalDeviceProperties2 deviceProperties2 = {};
            deviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
            VkPhysicalDeviceRayTracingPipelinePropertiesKHR
                raytracingPipelineProperties = {};
            raytracingPipelineProperties.sType =
                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
            deviceProperties2.pNext = &raytracingPipelineProperties;
            vkGetPhysicalDeviceProperties2(vkZero_core->physicalDevice,
                                        &deviceProperties2);

            VkDeviceSize handleSize =
                raytracingPipelineProperties.shaderGroupHandleSize;
            VkDeviceSize handleSizeAligned = ALIGN_UP(
                handleSize, raytracingPipelineProperties.shaderGroupBaseAlignment);
            sbtSize = handleSizeAligned * 2;

            VkBufferCreateInfo bufferInfo{};
            bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferInfo.size = sbtSize;
            bufferInfo.usage = VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR |
                            VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
            bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            if (vkCreateBuffer(vkZero_core->device, &bufferInfo, nullptr, &sbtBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to create buffer!");
            }

            VkMemoryRequirements memRequirements;
            vkGetBufferMemoryRequirements(vkZero_core->device, sbtBuffer, &memRequirements);

            VkMemoryAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = memRequirements.size;
            allocInfo.memoryTypeIndex = ResourceManager::findMemoryType(
                vkZero_core->physicalDevice, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

            if (vkAllocateMemory(vkZero_core->device, &allocInfo, nullptr, &sbtMemory) !=
                VK_SUCCESS) {
            throw std::runtime_error("failed to allocate buffer memory!");
            }

            vkBindBufferMemory(vkZero_core->device, sbtBuffer, sbtMemory, 0);

            std::vector<uint8_t> shaderHandleStorage(sbtSize);

            PFN_vkGetRayTracingShaderGroupHandlesKHR
                vkGetRayTracingShaderGroupHandlesKHR =
                    reinterpret_cast<PFN_vkGetRayTracingShaderGroupHandlesKHR>(
                        vkGetDeviceProcAddr(vkZero_core->device,
                                            "vkGetRayTracingShaderGroupHandlesKHR"));
            vkGetRayTracingShaderGroupHandlesKHR(vkZero_core->device,
                                                pipeline, 0, 2,
                                                sbtSize, shaderHandleStorage.data());

            void *mappedData;
            vkMapMemory(vkZero_core->device, sbtMemory, 0, sbtSize, 0,
                        &mappedData);
            memcpy(mappedData, shaderHandleStorage.data(), sbtSize);
            vkUnmapMemory(vkZero_core->device, sbtMemory);

            VkBufferDeviceAddressInfo bufferAddressInfo = {};
            bufferAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
            bufferAddressInfo.buffer = sbtBuffer;
            VkDeviceAddress sbtAddress = vkGetBufferDeviceAddress(
                vkZero_core->device, &bufferAddressInfo);

            raygenRegion.deviceAddress = sbtAddress;
            raygenRegion.stride = handleSizeAligned;
            raygenRegion.size = handleSizeAligned;

            missRegion.deviceAddress = sbtAddress + handleSizeAligned;
            missRegion.stride = handleSizeAligned;
            missRegion.size = handleSizeAligned;

            hitRegion = {};
            callableRegion = {};
        }
        
        ShaderGroupImpl* m_shaderGroup;
        VkRayTracingPipelineCreateInfoKHR pipelineInfo{};
        VkPipelineLayout pipelineLayout;
        VkPipeline pipeline = VK_NULL_HANDLE;

        VkStridedDeviceAddressRegionKHR raygenRegion;
        VkStridedDeviceAddressRegionKHR missRegion;
        VkStridedDeviceAddressRegionKHR hitRegion;
        VkStridedDeviceAddressRegionKHR callableRegion;
        VkDeviceSize sbtSize;
        VkBuffer sbtBuffer;
        VkDeviceMemory sbtMemory;

        std::tuple<ShaderResourcesBindings&...> resources;

        void bindResources(VkCommandBuffer commandBuffer, int currentFrame)
        {
            std::apply([&](auto& resource){
                vkCmdBindDescriptorSets(
                    commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
                    pipelineLayout, 0, 1,
                    &resource.impl->descriptorSets[currentFrame], 0, nullptr);
            }, resources);
        }

        friend class Raytracer;
    };
}
