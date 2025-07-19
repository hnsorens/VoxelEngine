#include "CommandManager.hpp"

CommandManager::CommandManager() {}
CommandManager::~CommandManager() {}
void CommandManager::createCommandPool(VkDevice device, uint32_t queueFamilyIndex) {}
void CommandManager::allocateCommandBuffers(VkDevice device, uint32_t count) {}
VkCommandPool CommandManager::getCommandPool() const { return VK_NULL_HANDLE; }
const std::vector<VkCommandBuffer>& CommandManager::getCommandBuffers() const { static std::vector<VkCommandBuffer> dummy; return dummy; } 