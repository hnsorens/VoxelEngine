#include "PipelineManager.hpp"

PipelineManager::PipelineManager() {}
PipelineManager::~PipelineManager() {}
void PipelineManager::createGraphicsPipeline(VkDevice device, VkRenderPass renderPass) {}
void PipelineManager::createRaytracingPipeline(VkDevice device) {}
VkPipeline PipelineManager::getGraphicsPipeline() const { return VK_NULL_HANDLE; }
VkPipeline PipelineManager::getRaytracingPipeline() const { return VK_NULL_HANDLE; }
VkPipelineLayout PipelineManager::getGraphicsPipelineLayout() const { return VK_NULL_HANDLE; }
VkPipelineLayout PipelineManager::getRaytracingPipelineLayout() const { return VK_NULL_HANDLE; } 