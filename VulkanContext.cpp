#include "VulkanContext.hpp"

VulkanContext::VulkanContext() {}
VulkanContext::~VulkanContext() {}
void VulkanContext::init(GLFWwindow* window) {}
void VulkanContext::cleanup() {}
void VulkanContext::recreateSwapChain() {}
VkDevice VulkanContext::getDevice() const { return VK_NULL_HANDLE; }
VkPhysicalDevice VulkanContext::getPhysicalDevice() const { return VK_NULL_HANDLE; }
VkInstance VulkanContext::getInstance() const { return VK_NULL_HANDLE; }
VkQueue VulkanContext::getGraphicsQueue() const { return VK_NULL_HANDLE; }
VkQueue VulkanContext::getPresentQueue() const { return VK_NULL_HANDLE; }
VkSwapchainKHR VulkanContext::getSwapChain() const { return VK_NULL_HANDLE; }
VkExtent2D VulkanContext::getSwapChainExtent() const { return {}; }
VkFormat VulkanContext::getSwapChainImageFormat() const { return {}; }
const std::vector<VkImage>& VulkanContext::getSwapChainImages() const { static std::vector<VkImage> dummy; return dummy; }
const std::vector<VkImageView>& VulkanContext::getSwapChainImageViews() const { static std::vector<VkImageView> dummy; return dummy; }
const std::vector<VkFramebuffer>& VulkanContext::getSwapChainFramebuffers() const { static std::vector<VkFramebuffer> dummy; return dummy; } 