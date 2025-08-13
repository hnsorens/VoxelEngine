#include "Engine.hpp"

// Static member definitions
std::unique_ptr<WindowManager> VoxelEngine::windowManager;
std::unique_ptr<VulkanContext> VoxelEngine::vulkanContext;
std::unique_ptr<SyncManager> VoxelEngine::syncManager;
std::unique_ptr<CommandManager> VoxelEngine::commandManager;
std::unique_ptr<PipelineManager> VoxelEngine::pipelineManager;
std::unique_ptr<Raytracer> VoxelEngine::raytracer;
std::unique_ptr<VoxelWorld> VoxelEngine::voxelWorld;
std::unique_ptr<Camera> VoxelEngine::camera;
std::unique_ptr<GlobalShaderTypes> VoxelEngine::shaders;
std::vector<VkCommandBuffer> VoxelEngine::raytracingCommandBuffers;
uint32_t VoxelEngine::currentFrame = 0;
uint8_t VoxelEngine::section = 0;

// Raytracing regions
VkStridedDeviceAddressRegionKHR VoxelEngine::raygenRegion{};
VkStridedDeviceAddressRegionKHR VoxelEngine::missRegion{};
VkStridedDeviceAddressRegionKHR VoxelEngine::hitRegion{};
VkStridedDeviceAddressRegionKHR VoxelEngine::callableRegion{};
VkDeviceSize VoxelEngine::sbtSize = 0;
VkBuffer VoxelEngine::sbtBuffer = VK_NULL_HANDLE;
VkDeviceMemory VoxelEngine::sbtMemory = VK_NULL_HANDLE; 