#pragma once
#include "WindowManager.hpp"
#include "VulkanContext.hpp"
#include "PipelineManager.hpp"
#include "CommandManager.hpp"
#include "SyncManager.hpp"
#include "Camera.hpp"
#include "VoxelWorld.hpp"
#include "Raytracer.hpp"
#include "ResourceManager.hpp"

class App {
public:
    App();
    ~App();

    void run();

private:
    WindowManager* windowManager;
    VulkanContext* vulkanContext;
    PipelineManager* pipelineManager;
    CommandManager* commandManager;
    SyncManager* syncManager;
    Camera* camera;
    VoxelWorld* voxelWorld;
    Raytracer* raytracer;
    ResourceManager* resourceManager;
}; 