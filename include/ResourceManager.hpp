#pragma once
#include "CommandManager.hpp"
#include "VulkanContext.hpp"
#include <cstdint>
#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

class ResourceManager {
public:
  static void createBuffer(VkDevice device, VkPhysicalDevice physicalDevice,
                           VkDeviceSize size, VkBufferUsageFlags usage,
                           VkMemoryPropertyFlags properties, VkBuffer &buffer,
                           VkDeviceMemory &bufferMemory);
  static void createImage(VkDevice device, VkPhysicalDevice physicalDevice,
                          uint32_t width, uint32_t height, uint32_t depth, VkFormat format,
                          VkImageTiling tiling, VkImageUsageFlags usage,
                          VkMemoryPropertyFlags properties, VkImage &image,
                          VkDeviceMemory &imageMemory);
  static uint32_t findMemoryType(VkPhysicalDevice physicalDevice,
                                 uint32_t typeFilter,
                                 VkMemoryPropertyFlags properties);
  static void createImageView(uint32_t depth, VkDevice device, VkFormat format, VkImage &image,
                              VkImageView &imageView);
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