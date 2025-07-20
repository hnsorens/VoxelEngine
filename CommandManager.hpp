#pragma once
#include "VulkanContext.hpp"
#include <vulkan/vulkan.h>
#include <vector>

class CommandManager {
public:
    CommandManager(std::unique_ptr<VulkanContext>& vulkanContext);
    ~CommandManager();

    void createCommandPool(VkDevice device, const QueueFamilyIndices& queueFamilyIndices);
    void allocateCommandBuffers(VkDevice device);
    VkCommandPool getCommandPool() const;
    const std::vector<VkCommandBuffer>& getCommandBuffers() const;

private:
    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;
}; 