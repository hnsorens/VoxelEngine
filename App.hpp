#pragma once
#include "Camera.hpp"
#include "CommandManager.hpp"
#include "PipelineManager.hpp"
#include "Raytracer.hpp"
#include "ResourceManager.hpp"
#include "SyncManager.hpp"
#include "VoxelWorld.hpp"
#include "VulkanContext.hpp"
#include "WindowManager.hpp"

class App {
public:
  App();
  ~App();

  void run();

private:
  WindowManager *windowManager;
  VulkanContext *vulkanContext;
  PipelineManager *pipelineManager;
  CommandManager *commandManager;
  SyncManager *syncManager;
  Camera *camera;
  VoxelWorld *voxelWorld;
  Raytracer *raytracer;
  ResourceManager *resourceManager;
};