#pragma once
#include "CommandManager.hpp"
#include "VkZero/context.hpp"
#include <cstdint>
#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

namespace VkZero
{
  class ResourceManager {
  public:
    static void createBuffer(VkDevice device, VkPhysicalDevice physicalDevice,
                            VkDeviceSize size, VkBufferUsageFlags usage,
                            VkMemoryPropertyFlags properties, VkBuffer &buffer,
                            VkDeviceMemory &bufferMemory);
    static uint32_t findMemoryType(VkPhysicalDevice physicalDevice,
                                  uint32_t typeFilter,
                                  VkMemoryPropertyFlags properties);
    static void
    transitionImageLayout(VkCommandBuffer commandBuffer,
                          VkImage image, VkFormat format, VkImageLayout oldLayout,
                          VkImageLayout newLayout, uint32_t mipLevels);
    static void transitionImageLayout(std::unique_ptr<CommandManager> &commandManager,
                                    std::unique_ptr<VulkanContext> &vulkanContext,
                                    VkImage image, VkFormat format, VkImageLayout oldLayout,
                                    VkImageLayout newLayout, uint32_t mipLevels);
    static std::vector<char> readFile(const std::string &filename);
  };  
}