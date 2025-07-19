#pragma once
#include <vulkan/vulkan.h>

class PipelineManager {
public:
    PipelineManager();
    ~PipelineManager();

    void createGraphicsPipeline(VkDevice device, VkRenderPass renderPass);
    void createRaytracingPipeline(VkDevice device);
    VkPipeline getGraphicsPipeline() const;
    VkPipeline getRaytracingPipeline() const;
    VkPipelineLayout getGraphicsPipelineLayout() const;
    VkPipelineLayout getRaytracingPipelineLayout() const;

private:
    VkPipeline graphicsPipeline;
    VkPipelineLayout graphicsPipelineLayout;
    VkPipeline raytracingPipeline;
    VkPipelineLayout raytracingPipelineLayout;
}; 