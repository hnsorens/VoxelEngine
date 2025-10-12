#include "Engine.hpp"
#include "Camera.hpp"
#include "VkZero/Internal/core_internal.hpp"
#include "VkZero/frame.hpp"
#include "VkZeroObjects.hpp"
#include "VoxelWorld.hpp"
#include "shaders.hpp"
#include <chrono>
#include <cstdio>
#include <iostream> // Added for debug output
#include <memory>
#include <thread>

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

void VoxelEngine::initWindow() {}

void VoxelEngine::initVulkan() {
  VkZero::vkZero_core = new VkZero::VkZeroCoreImpl_T();
  Window = std::make_unique<VkZero::Window>(WIDTH, HEIGHT, "Voxel Engine");

  shaders = std::make_unique<GlobalShaderLibrary>();

  camera = std::make_unique<Camera>(Window);

  voxelWorld = std::make_unique<VoxelWorld>();
  obj = std::make_unique<VkZeroObjects>(voxelWorld, camera, Window,
                                        [&](void* cb, uint32_t cf) {
                                          voxelWorld->updateVoxels((VkCommandBuffer)cb, cf);
                                        });
}

void VoxelEngine::mainLoop() {
  const double targetFrameTime = 1.0 / 120.0; // 60 FPS target
  double lastFrameTime = 0.0;
  uint64_t frameCount = 0;
  double lastFPSUpdate = 0.0;

  std::cout << "Starting main loop. Press ESC to exit." << std::endl;

  while (!Window->shouldClose()) {

    Window->pollEvents();

    // Check if window is minimized
    int width, height;
    Window->getFramebufferSize(&width, &height);
    if (width > 0 && height > 0) {
      drawFrame();
      frameCount++;
    }
  }

  std::cout << "Shutting down..." << std::endl;
  vkDeviceWaitIdle(VkZero::vkZero_core->device);
}

void VoxelEngine::drawFrame() {
  // Wait for the previous frame to finish with a timeout
  camera->update(Window, voxelWorld, obj->frame.getFrame());
  obj->draw();
}
