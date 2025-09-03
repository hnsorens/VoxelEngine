#include "Engine.hpp"
#include "Camera.hpp"
#include "PipelineManager.hpp"
#include "Raytracer.hpp"
#include "SyncManager.hpp"
#include "VoxelWorld.hpp"
#include "VkZero/window.hpp"
#include "shaders.hpp"
#include <cstdio>
#include <memory>
#include <thread>
#include <chrono>
#include <iostream> // Added for debug output

// Static member definitions
std::unique_ptr<VkZero::WindowManager> VoxelEngine::windowManager;
std::unique_ptr<VkZero::VulkanContext> VoxelEngine::vulkanContext;
std::unique_ptr<SyncManager> VoxelEngine::syncManager;
std::unique_ptr<CommandManager> VoxelEngine::commandManager;
std::unique_ptr<PipelineManager> VoxelEngine::pipelineManager;
std::unique_ptr<Raytracer> VoxelEngine::raytracer;
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
    
    vulkanContext = std::make_unique<VkZero::VulkanContext>();
    windowManager = std::make_unique<VkZero::WindowManager>(vulkanContext, WIDTH, HEIGHT, "Voxel Engine");

    shaders = std::make_unique<GlobalShaderLibrary>(vulkanContext);

    camera = std::make_unique<Camera>(vulkanContext, windowManager);
    commandManager = std::make_unique<CommandManager>(vulkanContext);

    createCommandBuffers();

    voxelWorld = std::make_unique<VoxelWorld>(vulkanContext, commandManager);

    raytracer = std::make_unique<Raytracer>(commandManager, vulkanContext,
                                            voxelWorld, camera);
    pipelineManager =
        std::make_unique<PipelineManager>(vulkanContext, raytracer, windowManager);

    syncManager = std::make_unique<SyncManager>(vulkanContext);
}

void VoxelEngine::mainLoop() {
    const double targetFrameTime = 1.0 / 60.0; // 60 FPS target
    double lastFrameTime = 0.0;
    uint64_t frameCount = 0;
    double lastFPSUpdate = 0.0;
    
    std::cout << "Starting main loop. Press ESC to exit." << std::endl;
    
    while (!windowManager->shouldClose()) {
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
        
        windowManager->pollEvents();
        
        // Check if window is minimized
        int width, height;
        windowManager->getFramebufferSize(&width, &height);
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
    vkDeviceWaitIdle(vulkanContext->getDevice());
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
            vulkanContext->getDevice(), &raytracingAllocInfo,
            raytracingCommandBuffers.data()) != VK_SUCCESS) {
      throw std::runtime_error("failed to allocate command buffers!");
    }
  }

  void VoxelEngine::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
      throw std::runtime_error("failed to begin recording command buffer!");
    }
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = pipelineManager->getRenderPass();
    renderPassInfo.framebuffer = pipelineManager->getFrameBuffer(imageIndex);
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = windowManager->getSwapChainExtent();
    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo,
                         VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      pipelineManager->getGraphicsPipeline());
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)windowManager->getSwapChainExtent().width;
    viewport.height = (float)windowManager->getSwapChainExtent().height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = windowManager->getSwapChainExtent();
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipelineManager->getGraphicsPipelineLayout(), 0, 1,
                            &pipelineManager->getDescriptorSet(currentFrame), 0,
                            nullptr);
    vkCmdDraw(commandBuffer, 6, 1, 0, 0);
    vkCmdEndRenderPass(commandBuffer);
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
      throw std::runtime_error("failed to record command buffer!");
    }
  }

  void VoxelEngine::drawFrame() {
    // Wait for the previous frame to finish with a timeout
    VkResult fenceResult = vkWaitForFences(vulkanContext->getDevice(), 1,
                    &syncManager->getInFlightFences()[currentFrame], VK_TRUE,
                    1000000000); // 1 second timeout
    
    if (fenceResult == VK_TIMEOUT) {
        std::cout << "Warning: Frame fence timeout, continuing anyway" << std::endl;
    }
    
    camera->update(windowManager, voxelWorld, currentFrame);
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(
        vulkanContext->getDevice(), windowManager->getSwapChain(), UINT64_MAX,
        syncManager->getImageAvailableSemaphores()[currentFrame],
        VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
      windowManager->recreateSwapchain(vulkanContext);
      pipelineManager->recreateFramebuffers(vulkanContext, windowManager);
      return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
      throw std::runtime_error("failed to acquire swap chain image!");
    }
    vkResetFences(vulkanContext->getDevice(), 1,
                  &syncManager->getInFlightFences()[currentFrame]);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    
    vkResetCommandBuffer(commandManager->getCommandBuffers()[currentFrame], 0);
    if (vkBeginCommandBuffer(commandManager->getCommandBuffers()[currentFrame], &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    pipelineManager->renderPass.record(commandManager->getCommandBuffers()[currentFrame], windowManager, currentFrame, imageIndex);

    if (vkEndCommandBuffer(commandManager->getCommandBuffers()[currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to record raytracing command buffer!");
    }

    // recordVoxelCommandBuffer(
    //     raytracingCommandBuffers[currentFrame],
    //     currentFrame,
    //     section);
    vkResetCommandBuffer(raytracingCommandBuffers[currentFrame], 0);
    if (vkBeginCommandBuffer(raytracingCommandBuffers[currentFrame], &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    raytracer->renderPass.record(raytracingCommandBuffers[currentFrame], currentFrame, imageIndex);
    voxelWorld->updateVoxels(raytracingCommandBuffers[currentFrame], vulkanContext, currentFrame);

    if (vkEndCommandBuffer(raytracingCommandBuffers[currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to record raytracing command buffer!");
    }





    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {
        syncManager->getImageAvailableSemaphores()[currentFrame]};
    VkPipelineStageFlags waitStages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    std::vector<VkCommandBuffer> commands;
    commands.push_back(
        raytracingCommandBuffers[currentFrame]);
    commands.push_back(commandManager->getCommandBuffers()[currentFrame]);

    submitInfo.commandBufferCount = 2;
    submitInfo.pCommandBuffers = commands.data();
    VkSemaphore signalSemaphores[] = {
        syncManager->getRenderFinishedSemaphores()[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(vulkanContext->getGraphicsQueue(), 1, &submitInfo,
                      syncManager->getInFlightFences()[currentFrame]) !=
        VK_SUCCESS) {
      throw std::runtime_error("failed to submit draw command buffer!");
    }
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {windowManager->getSwapChain()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(vulkanContext->getPresentQueue(), &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
        windowManager->framebufferResized) {
      windowManager->framebufferResized = false;
      std::cout << "Recreating swapchain..." << std::endl;
      windowManager->recreateSwapchain(vulkanContext);
      pipelineManager->recreateFramebuffers(vulkanContext, windowManager);
      // Add a small delay to prevent excessive recreation
      std::this_thread::sleep_for(std::chrono::milliseconds(16));
    } else if (result != VK_SUCCESS) {
      throw std::runtime_error("failed to present swap chain image!");
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    if (currentFrame == 0)
      section = (section + 1) % 16;
  }