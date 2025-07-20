#include "CommandManager.hpp"
#include "VulkanContext.hpp"
#include <memory>
#include <stdexcept>

CommandManager::CommandManager(std::unique_ptr<VulkanContext>& vulkanContext) 
{
    createCommandPool(vulkanContext->getDevice(), vulkanContext->getQueueFamilyIndices());
    allocateCommandBuffers(vulkanContext->getDevice());
}

CommandManager::~CommandManager() {}

void CommandManager::createCommandPool(VkDevice device, const QueueFamilyIndices& queueFamilyIndices) 
{
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    }
}

void CommandManager::allocateCommandBuffers(VkDevice device) 
{
    commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t) commandBuffers.size();

    if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

VkCommandPool CommandManager::getCommandPool() const { return commandPool; }

const std::vector<VkCommandBuffer>& CommandManager::getCommandBuffers() const { return commandBuffers; } 