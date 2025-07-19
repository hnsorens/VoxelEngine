#pragma once
#include <vulkan/vulkan.h>
#include <vector>

class CommandManager {
public:
    CommandManager();
    ~CommandManager();

    void createCommandPool(VkDevice device, uint32_t queueFamilyIndex);
    void allocateCommandBuffers(VkDevice device, uint32_t count);
    VkCommandPool getCommandPool() const;
    const std::vector<VkCommandBuffer>& getCommandBuffers() const;

private:
    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;
}; 