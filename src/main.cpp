#include "Camera.hpp"
#include "Engine.hpp"
#include "VkZero/vk_zero.hpp"
#include "VkZero/window.hpp"
#include "VkZeroObjects.hpp"
#include "VoxelWorld.hpp"

int main() {

  VkZero::VkZeroInit();
  VkZero::Window window{1920, 1280, "Voxel Engine"};
  VoxelWorld world;
  Camera camera{window};
  VkZeroObjects objects{world, camera, window, [&](void* commandBuffer, uint32_t currentFrame){
    world.updateVoxels(commandBuffer, currentFrame);
  }};
  
  while (!window.shouldClose())
  {
    window.pollEvents();

    // Check if window is minimized
    int width, height;
    window.getFramebufferSize(&width, &height);
    if (width > 0 && height > 0) {
      camera.update(window, world, objects.frame.getFrame());
      objects.draw();
    }
  }
}