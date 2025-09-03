#include "VkZero/context.hpp"
#include "shaders.hpp"
#include <cstdio>
#include <memory>
#include <iostream>

static float semiRandomFloat(float x, float y, float z);

const uint32_t WIDTH = 1920;
const uint32_t HEIGHT = 1280;

#define RAYTRACE_HEIGHT 1080
#define RAYTRACE_WIDTH 1920

namespace VkZero {
  class VulkanContext;
  class WindowManager;
}
class VoxelEngine {
public:
  static void run();

  template <VkZero::FixedString name>
  static auto& get_shader()
  {
    return shaders->get<name>();
  }

  static std::unique_ptr<VkZero::WindowManager> windowManager;
  static std::unique_ptr<VkZero::VulkanContext> vulkanContext;
  static std::unique_ptr<class SyncManager> syncManager;
  static std::unique_ptr<class CommandManager> commandManager;
  static std::unique_ptr<class PipelineManager> pipelineManager;
  static std::unique_ptr<class Raytracer> raytracer;
  static std::unique_ptr<class VoxelWorld> voxelWorld;
  static std::unique_ptr<class Camera> camera;

  static std::unique_ptr<GlobalShaderLibrary> shaders;

  static std::vector<VkCommandBuffer> raytracingCommandBuffers;

  static uint32_t currentFrame;
  static uint8_t section;

  static void initWindow();
  static void initVulkan();
  static void mainLoop();

#define ALIGN_UP(value, alignment)                                             \
  (((value) + (alignment) - 1) & ~((alignment) - 1))

  static void createBuffer(VkDevice device, VkPhysicalDevice physicalDevice,
                    VkDeviceSize size, VkBufferUsageFlags usage,
                    VkMemoryPropertyFlags properties, VkBuffer &buffer,
                    VkDeviceMemory &bufferMemory);
  static void createCommandBuffers();

  static void recordVoxelCommandBuffer(VkCommandBuffer commandBuffer,
                                uint32_t imageIndex, uint8_t section);
  static void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
  static void drawFrame();
  static VkZero::SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

  static VKAPI_ATTR VkBool32 VKAPI_CALL
  debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                VkDebugUtilsMessageTypeFlagsEXT messageType,
                const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                void *pUserData) {
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
    fflush(stderr);

    return VK_FALSE;
  }

  friend class DescriptorPool;
};

static float semiRandomFloat(float x, float y, float z) {
  // A simple hashing function using the input vector components
  uint32_t hash = x * 123456789 + y * 987654321 + z * 567890123;

  // A bitwise operation to mix the hash value
  hash = (hash ^ (hash >> 21)) * 2654435761u;
  hash = hash ^ (hash >> 21);
  hash = hash * 668265263;
  hash = hash ^ (hash >> 21);

  // Return a float between 0 and 1 based on the hash
  return (float)(hash & 0xFFFFFFF) / (float)0xFFFFFFF;
}