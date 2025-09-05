#pragma once
#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

namespace VkZero
{
  struct QueueFamilyIndices;
}
class CommandManager {
public:
  CommandManager();
  ~CommandManager();

  VkCommandBuffer
  beginSingleTimeCommands();
  void endSingleTimeCommands(VkCommandBuffer commandBuffer);
  VkCommandPool getCommandPool() const;
  const std::vector<VkCommandBuffer> &getCommandBuffers() const;

private:
  void createCommandPool();
  void allocateCommandBuffers();

  VkCommandPool commandPool;
  std::vector<VkCommandBuffer> commandBuffers;
};