#include "CommandManager.hpp"
#include "VkZero/context.hpp"
#include "VkZero/info.hpp"
#include <memory>
#include <stdexcept>

CommandManager::CommandManager(std::unique_ptr<VkZero::VulkanContext> &vulkanContext) {
  createCommandPool(vulkanContext->getDevice(),
                    vulkanContext->getQueueFamilyIndices());
  allocateCommandBuffers(vulkanContext->getDevice());
}

CommandManager::~CommandManager() {}

void CommandManager::createCommandPool(
    VkDevice device, const VkZero::QueueFamilyIndices &queueFamilyIndices) {
  VkCommandPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

  if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create command pool!");
  }
}

void CommandManager::allocateCommandBuffers(VkDevice device) {
  commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.commandPool = commandPool;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

  if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to allocate command buffers!");
  }
}

VkCommandBuffer CommandManager::beginSingleTimeCommands(
    std::unique_ptr<VkZero::VulkanContext> &vulkanContext) {
  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = commandPool;
  allocInfo.commandBufferCount = 1;

  VkCommandBuffer commandBuffer;
  vkAllocateCommandBuffers(vulkanContext->getDevice(), &allocInfo,
                           &commandBuffer);

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(commandBuffer, &beginInfo);

  return commandBuffer;
}

void CommandManager::endSingleTimeCommands(
    std::unique_ptr<VkZero::VulkanContext> &vulkanContext,
    VkCommandBuffer commandBuffer) {
  vkEndCommandBuffer(commandBuffer);

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;

  vkQueueSubmit(vulkanContext->getGraphicsQueue(), 1, &submitInfo,
                VK_NULL_HANDLE);
  vkQueueWaitIdle(vulkanContext->getGraphicsQueue());

  vkFreeCommandBuffers(vulkanContext->getDevice(), commandPool, 1,
                       &commandBuffer);
}

VkCommandPool CommandManager::getCommandPool() const { return commandPool; }

const std::vector<VkCommandBuffer> &CommandManager::getCommandBuffers() const {
  return commandBuffers;
}