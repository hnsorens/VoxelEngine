#pragma once
#include <vulkan/vulkan.h>
#include <vector>

class Raytracer {
public:
    Raytracer();
    ~Raytracer();

    void createRaytracingPipeline(VkDevice device);
    void createRaytracingResources(VkDevice device);
    void recordRaytracingCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, uint8_t section);

private:
    VkPipeline raytracingPipeline;
    VkPipelineLayout raytracingPipelineLayout;
    std::vector<VkCommandBuffer> raytracingCommandBuffers;
}; 