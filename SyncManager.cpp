#include "SyncManager.hpp"

SyncManager::SyncManager() {}
SyncManager::~SyncManager() {}
void SyncManager::createSyncObjects(VkDevice device, uint32_t maxFramesInFlight) {}
const std::vector<VkSemaphore>& SyncManager::getImageAvailableSemaphores() const { static std::vector<VkSemaphore> dummy; return dummy; }
const std::vector<VkSemaphore>& SyncManager::getRenderFinishedSemaphores() const { static std::vector<VkSemaphore> dummy; return dummy; }
const std::vector<VkFence>& SyncManager::getInFlightFences() const { static std::vector<VkFence> dummy; return dummy; } 