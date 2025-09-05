#pragma once
#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

class SyncManager {
public:
  SyncManager();
  ~SyncManager();

  const std::vector<VkSemaphore> &getImageAvailableSemaphores() const;
  const std::vector<VkSemaphore> &getRenderFinishedSemaphores() const;
  const std::vector<VkFence> &getInFlightFences() const;

private:
  std::vector<VkSemaphore> imageAvailableSemaphores;
  std::vector<VkSemaphore> renderFinishedSemaphores;
  std::vector<VkFence> inFlightFences;
};