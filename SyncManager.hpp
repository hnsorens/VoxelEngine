#pragma once
#include "VulkanContext.hpp"
#include <memory>
#include <vulkan/vulkan.h>
#include <vector>

class SyncManager {
public:
    SyncManager(std::unique_ptr<VulkanContext>& vulkanContext);
    ~SyncManager();

    void createSyncObjects(VkDevice device);
    const std::vector<VkSemaphore>& getImageAvailableSemaphores() const;
    const std::vector<VkSemaphore>& getRenderFinishedSemaphores() const;
    const std::vector<VkFence>& getInFlightFences() const;

private:
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
}; 