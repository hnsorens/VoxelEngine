#pragma once
#include <vulkan/vulkan.h>
#include <vector>

class SyncManager {
public:
    SyncManager();
    ~SyncManager();

    void createSyncObjects(VkDevice device, uint32_t maxFramesInFlight);
    const std::vector<VkSemaphore>& getImageAvailableSemaphores() const;
    const std::vector<VkSemaphore>& getRenderFinishedSemaphores() const;
    const std::vector<VkFence>& getInFlightFences() const;

private:
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
}; 