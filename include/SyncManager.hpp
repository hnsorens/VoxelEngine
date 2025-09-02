#pragma once
#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

namespace VkZero{
  class VulkanContext;
};

class SyncManager {
public:
  SyncManager(std::unique_ptr<VkZero::VulkanContext> &vulkanContext);
  ~SyncManager();

  void createSyncObjects(VkDevice device);
  const std::vector<VkSemaphore> &getImageAvailableSemaphores() const;
  const std::vector<VkSemaphore> &getRenderFinishedSemaphores() const;
  const std::vector<VkFence> &getInFlightFences() const;

private:
  std::vector<VkSemaphore> imageAvailableSemaphores;
  std::vector<VkSemaphore> renderFinishedSemaphores;
  std::vector<VkFence> inFlightFences;
};