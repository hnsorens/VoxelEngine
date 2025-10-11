#include "Engine.hpp"
#include "Camera.hpp"
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

    voxelWorld = std::make_unique<VoxelWorld>();
    obj = std::make_unique<VkZeroObjects>(
                                            voxelWorld, camera, Window, [&](VkCommandBuffer cb, uint32_t cf){voxelWorld->updateVoxels(cb, cf);});
}

void VoxelEngine::mainLoop() {
    const double targetFrameTime = 1.0 / 120.0; // 60 FPS target
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





  void VoxelEngine::drawFrame() {
    // Wait for the previous frame to finish with a timeout
    camera->update(Window, voxelWorld, obj->frame.impl->currentFrame);
    obj->draw();
  }
