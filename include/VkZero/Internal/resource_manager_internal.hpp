#pragma once
#include <cstdint>
#include <memory>
#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace VkZero
{
  class ResourceManager {
  public:
    static uint32_t findMemoryType(VkPhysicalDevice physicalDevice,
                                  uint32_t typeFilter,
                                  VkMemoryPropertyFlags properties);
    static void
    transitionImageLayout(VkCommandBuffer commandBuffer,
                          VkImage image, VkFormat format, VkImageLayout oldLayout,
                          VkImageLayout newLayout, uint32_t mipLevels);
    static void transitionImageLayout(
                                    VkImage image, VkFormat format, VkImageLayout oldLayout,
                                    VkImageLayout newLayout, uint32_t mipLevels);
    static std::vector<char> readFile(const std::string &filename);
    static VkCommandBuffer beginSingleTimeCommands();
    static void endSingleTimeCommands(VkCommandBuffer commandBuffer);
  };  
}
