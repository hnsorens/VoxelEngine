#pragma once
#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

namespace VkZero
{
  class VulkanContext;
  struct QueueFamilyIndices;
}
class CommandManager {
public:
  CommandManager(std::unique_ptr<VkZero::VulkanContext> &vulkanContext);
  ~CommandManager();

  VkCommandBuffer
  beginSingleTimeCommands(std::unique_ptr<VkZero::VulkanContext> &vulkanContext);
  void endSingleTimeCommands(std::unique_ptr<VkZero::VulkanContext> &vulkanContext,
                             VkCommandBuffer commandBuffer);
  VkCommandPool getCommandPool() const;
  const std::vector<VkCommandBuffer> &getCommandBuffers() const;

private:
  void createCommandPool(VkDevice device,
                         const VkZero::QueueFamilyIndices &queueFamilyIndices);
  void allocateCommandBuffers(VkDevice device);

  VkCommandPool commandPool;
  std::vector<VkCommandBuffer> commandBuffers;
};