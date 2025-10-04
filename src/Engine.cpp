#include "Engine.hpp"
#include "Camera.hpp"
#include "PipelineManager.hpp"
#include "Raytracer.hpp"
#include "SyncManager.hpp"
#include "VkZero/Internal/core_internal.hpp"
#include "VkZero/frame.hpp"
#include "VkZeroObjects.hpp"
#include "VoxelWorld.hpp"
#include "VkZero/Internal/window_internal.hpp"
#include "VkZero/Internal/graphics_renderpass_internal.hpp"
#include "VkZero/Internal/raytracing_renderpass_internal.hpp"
#include "shaders.hpp"
#include <cstdio>
#include <memory>
#include <thread>
#include <chrono>
#include <iostream> // Added for debug output

// Static member definitions
std::unique_ptr<VkZero::Window> VoxelEngine::Window;
std::unique_ptr<SyncManager> VoxelEngine::syncManager;
std::unique_ptr<CommandManager> VoxelEngine::commandManager;
std::unique_ptr<VkZeroObjects> VoxelEngine::obj;
std::unique_ptr<VoxelWorld> VoxelEngine::voxelWorld;
std::unique_ptr<Camera> VoxelEngine::camera;
std::unique_ptr<GlobalShaderLibrary> VoxelEngine::shaders;
std::vector<VkCommandBuffer> VoxelEngine::raytracingCommandBuffers;
uint32_t VoxelEngine::currentFrame = 0;
uint8_t VoxelEngine::section = 0;

void VoxelEngine::run() {
    initWindow();
    initVulkan();
    mainLoop();
}

void VoxelEngine::initWindow() {
    
}

void VoxelEngine::initVulkan() {
    VkZero::vkZero_core = new VkZero::VkZeroCoreImpl_T();
    Window = std::make_unique<VkZero::Window>(WIDTH, HEIGHT, "Voxel Engine");

    shaders = std::make_unique<GlobalShaderLibrary>();

    camera = std::make_unique<Camera>(Window);
    commandManager = std::make_unique<CommandManager>();

    createCommandBuffers();
    voxelWorld = std::make_unique<VoxelWorld>(commandManager);
    obj = std::make_unique<VkZeroObjects>(commandManager,
                                            voxelWorld, camera, Window, [&](VkCommandBuffer cb, uint32_t cf){voxelWorld->updateVoxels(cb, cf);});
    syncManager = std::make_unique<SyncManager>();
}

void VoxelEngine::mainLoop() {
    const double targetFrameTime = 1.0 / 60.0; // 60 FPS target
    double lastFrameTime = 0.0;
    uint64_t frameCount = 0;
    double lastFPSUpdate = 0.0;
    
    std::cout << "Starting main loop. Press ESC to exit." << std::endl;
    
    while (!Window->impl->shouldClose()) {
        double currentTime = glfwGetTime();
        double deltaTime = currentTime - lastFrameTime;
        
        // Limit frame rate
        if (deltaTime < targetFrameTime) {
            double sleepTime = targetFrameTime - deltaTime;
            if (sleepTime > 0.001) { // Only sleep if more than 1ms
                std::this_thread::sleep_for(std::chrono::duration<double>(sleepTime));
            }
            continue;
        }
        
        Window->impl->pollEvents();
        
        // Check if window is minimized
        int width, height;
        Window->getFramebufferSize(&width, &height);
        if (width > 0 && height > 0) {
            drawFrame();
            frameCount++;
        }
        
        lastFrameTime = currentTime;
        
        // Print FPS every second
        if (currentTime - lastFPSUpdate >= 1.0) {
            std::cout << "FPS: " << frameCount << std::endl;
            frameCount = 0;
            lastFPSUpdate = currentTime;
        }
        
        // Add a small delay to prevent excessive CPU usage
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }

    std::cout << "Shutting down..." << std::endl;
    vkDeviceWaitIdle(VkZero::vkZero_core->device);
}



































void VoxelEngine::createBuffer(VkDevice device, VkPhysicalDevice physicalDevice,
                    VkDeviceSize size, VkBufferUsageFlags usage,
                    VkMemoryPropertyFlags properties, VkBuffer &buffer,
                    VkDeviceMemory &bufferMemory) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
      throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = VkZero::ResourceManager::findMemoryType(
        physicalDevice, memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) !=
        VK_SUCCESS) {
      throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(device, buffer, bufferMemory, 0);
  }

  void VoxelEngine::createCommandBuffers() {
    raytracingCommandBuffers.clear(); // Clear the vector first
    raytracingCommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo raytracingAllocInfo{};
    raytracingAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    raytracingAllocInfo.commandPool = commandManager->getCommandPool();
    raytracingAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    raytracingAllocInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT; // Use constant directly

    if (vkAllocateCommandBuffers(
            VkZero::vkZero_core->device, &raytracingAllocInfo,
            raytracingCommandBuffers.data()) != VK_SUCCESS) {
      throw std::runtime_error("failed to allocate command buffers!");
    }
  }

  void VoxelEngine::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
  
  }

  void VoxelEngine::drawFrame() {
    // Wait for the previous frame to finish with a timeout
    camera->update(Window, voxelWorld, obj->frame.impl->currentFrame);
    obj->draw();
  }
