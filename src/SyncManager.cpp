#include "SyncManager.hpp"
#include "VkZero/context.hpp"
#include "VkZero/window.hpp"
#include <memory>
#include <stdexcept>

SyncManager::SyncManager(std::unique_ptr<VkZero::VulkanContext> &vulkanContext) {
  createSyncObjects(vulkanContext->getDevice());
}

SyncManager::~SyncManager() {}

void SyncManager::createSyncObjects(VkDevice device) {
  imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

  VkSemaphoreCreateInfo semaphoreInfo{};
  semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkFenceCreateInfo fenceInfo{};
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    if (vkCreateSemaphore(device, &semaphoreInfo, nullptr,
                          &imageAvailableSemaphores[i]) != VK_SUCCESS ||
        vkCreateSemaphore(device, &semaphoreInfo, nullptr,
                          &renderFinishedSemaphores[i]) != VK_SUCCESS ||
        vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) !=
            VK_SUCCESS) {
      throw std::runtime_error(
          "failed to create synchronization objects for a frame!");
    }
  }
}

const std::vector<VkSemaphore> &
SyncManager::getImageAvailableSemaphores() const {
  return imageAvailableSemaphores;
}
const std::vector<VkSemaphore> &
SyncManager::getRenderFinishedSemaphores() const {
  return renderFinishedSemaphores;
}
const std::vector<VkFence> &SyncManager::getInFlightFences() const {
  return inFlightFences;
}