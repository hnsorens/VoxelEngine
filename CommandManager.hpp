#pragma once
#include "VulkanContext.hpp"
#include <vulkan/vulkan.h>
#include <vector>

class CommandManager {
public:
    CommandManager(std::unique_ptr<VulkanContext>& vulkanContext);
    ~CommandManager();

    VkCommandBuffer beginSingleTimeCommands(std::unique_ptr<VulkanContext>& vulkanContext);
    void endSingleTimeCommands(std::unique_ptr<VulkanContext>& vulkanContext, VkCommandBuffer commandBuffer);
    VkCommandPool getCommandPool() const;
    const std::vector<VkCommandBuffer>& getCommandBuffers() const;

private:
    void createCommandPool(VkDevice device, const QueueFamilyIndices& queueFamilyIndices);
    void allocateCommandBuffers(VkDevice device);

    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;
}; 