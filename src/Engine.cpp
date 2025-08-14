#include "Engine.hpp"
#include "Camera.hpp"
#include "PipelineManager.hpp"
#include "Raytracer.hpp"
#include "SyncManager.hpp"
#include "VoxelWorld.hpp"
#include <memory>

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

void VoxelEngine::run() {
    initWindow();
    initVulkan();
    mainLoop();
}

void VoxelEngine::initWindow() {
    windowManager = std::make_unique<WindowManager>(WIDTH, HEIGHT, "Voxel Engine");
}

void VoxelEngine::initVulkan() {
    vulkanContext = std::make_unique<VulkanContext>();
    vulkanContext->init(windowManager);

    shaders = std::make_unique<GlobalShaderTypes>(vulkanContext);

    camera = std::make_unique<Camera>(vulkanContext);
    commandManager = std::make_unique<CommandManager>(vulkanContext);

    createCommandBuffers();

    voxelWorld = std::make_unique<VoxelWorld>(vulkanContext, commandManager);

    raytracer = std::make_unique<Raytracer>(commandManager, vulkanContext,
                                            voxelWorld, camera);
    pipelineManager =
        std::make_unique<PipelineManager>(vulkanContext, raytracer);

    syncManager = std::make_unique<SyncManager>(vulkanContext);

    createRaytracingRegions();
}

void VoxelEngine::mainLoop() {
    while (!windowManager->shouldClose()) {
        windowManager->pollEvents();
        drawFrame();
    }

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
    allocInfo.memoryTypeIndex = ResourceManager::findMemoryType(
        physicalDevice, memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) !=
        VK_SUCCESS) {
      throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(device, buffer, bufferMemory, 0);
  }

  void VoxelEngine::createRaytracingRegions() {
    VkPhysicalDeviceProperties2 deviceProperties2 = {};
    deviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    VkPhysicalDeviceRayTracingPipelinePropertiesKHR
        raytracingPipelineProperties = {};
    raytracingPipelineProperties.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
    deviceProperties2.pNext = &raytracingPipelineProperties;
    vkGetPhysicalDeviceProperties2(vulkanContext->getPhysicalDevice(),
                                   &deviceProperties2);

    VkDeviceSize handleSize =
        raytracingPipelineProperties.shaderGroupHandleSize;
    VkDeviceSize handleSizeAligned = ALIGN_UP(
        handleSize, raytracingPipelineProperties.shaderGroupBaseAlignment);
    sbtSize = handleSizeAligned * 2;

    createBuffer(vulkanContext->getDevice(), vulkanContext->getPhysicalDevice(),
                 sbtSize,
                 VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR |
                     VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 sbtBuffer, sbtMemory);

    std::vector<uint8_t> shaderHandleStorage(sbtSize);

    PFN_vkGetRayTracingShaderGroupHandlesKHR
        vkGetRayTracingShaderGroupHandlesKHR =
            reinterpret_cast<PFN_vkGetRayTracingShaderGroupHandlesKHR>(
                vkGetDeviceProcAddr(vulkanContext->getDevice(),
                                    "vkGetRayTracingShaderGroupHandlesKHR"));
    vkGetRayTracingShaderGroupHandlesKHR(vulkanContext->getDevice(),
                                         raytracer->getPipeline(), 0, 2,
                                         sbtSize, shaderHandleStorage.data());

    void *mappedData;
    vkMapMemory(vulkanContext->getDevice(), sbtMemory, 0, sbtSize, 0,
                &mappedData);
    memcpy(mappedData, shaderHandleStorage.data(), sbtSize);
    vkUnmapMemory(vulkanContext->getDevice(), sbtMemory);

    VkBufferDeviceAddressInfo bufferAddressInfo = {};
    bufferAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    bufferAddressInfo.buffer = sbtBuffer;
    VkDeviceAddress sbtAddress = vkGetBufferDeviceAddress(
        vulkanContext->getDevice(), &bufferAddressInfo);

    raygenRegion.deviceAddress = sbtAddress;
    raygenRegion.stride = handleSizeAligned;
    raygenRegion.size = handleSizeAligned;

    missRegion.deviceAddress = sbtAddress + handleSizeAligned;
    missRegion.stride = handleSizeAligned;
    missRegion.size = handleSizeAligned;

    hitRegion = {};
    callableRegion = {};
  }

  void VoxelEngine::createCommandBuffers() {
    raytracingCommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo raytracingAllocInfo{};
    raytracingAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    raytracingAllocInfo.commandPool = commandManager->getCommandPool();
    raytracingAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    raytracingAllocInfo.commandBufferCount =
        (uint32_t)commandManager->getCommandBuffers().size();

    if (vkAllocateCommandBuffers(
            vulkanContext->getDevice(), &raytracingAllocInfo,
            raytracingCommandBuffers.data()) != VK_SUCCESS) {
      throw std::runtime_error("failed to allocate command buffers!");
    }
  }


  void VoxelEngine::recordVoxelCommandBuffer(VkCommandBuffer commandBuffer,
                                uint32_t imageIndex, uint8_t section) {
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
      throw std::runtime_error("failed to begin recording command buffer!");
    }

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
                      raytracer->getPipeline());

    vkCmdBindDescriptorSets(
        commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
        raytracer->getPipelineLayout(), 0, 1,
        &raytracer->getDescriptorSet(currentFrame), 0, nullptr);

    PFN_vkCmdTraceRaysKHR vkCmdTraceRaysKHR =
        reinterpret_cast<PFN_vkCmdTraceRaysKHR>(vkGetDeviceProcAddr(
            vulkanContext->getDevice(), "vkCmdTraceRaysKHR"));

    PushConstant c;
    c.flag = 0;
    c.frame = 0;
    vkCmdPushConstants(commandBuffer, raytracer->getPipelineLayout(),
                       VK_SHADER_STAGE_RAYGEN_BIT_KHR, 0, 8, &c);
    vkCmdTraceRaysKHR(commandBuffer, &raygenRegion, &missRegion, &hitRegion,
                      &callableRegion, RAYTRACE_WIDTH, RAYTRACE_HEIGHT, 1);

    VkMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    barrier.srcAccessMask =
        VK_ACCESS_SHADER_WRITE_BIT |
        VK_ACCESS_SHADER_READ_BIT; // Ensure writes from the first trace finish
    barrier.dstAccessMask =
        VK_ACCESS_SHADER_READ_BIT |
        VK_ACCESS_SHADER_WRITE_BIT; // Ensure the second trace can read them
    barrier.pNext = 0;

    vkCmdPipelineBarrier(
        commandBuffer,
        VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR, // Source: First trace
                                                      // rays execution
        VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR, // Destination: Second
                                                      // trace rays execution
        0, 1, &barrier, 0, nullptr, 0, nullptr);

    c.flag = 1;
    vkCmdPushConstants(commandBuffer, raytracer->getPipelineLayout(),
                       VK_SHADER_STAGE_RAYGEN_BIT_KHR, 0, 4, &c);

    vkCmdTraceRaysKHR(commandBuffer, &raygenRegion, &missRegion, &hitRegion,
                      &callableRegion, RAYTRACE_WIDTH, RAYTRACE_HEIGHT, 1);
    vkCmdPipelineBarrier(
        commandBuffer,
        VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR, // Source: First trace
                                                      // rays execution
        VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR, // Destination: Second
                                                      // trace rays execution
        0, 1, &barrier, 0, nullptr, 0, nullptr);

    c.flag = 2;
    vkCmdPushConstants(commandBuffer, raytracer->getPipelineLayout(),
                       VK_SHADER_STAGE_RAYGEN_BIT_KHR, 0, 4, &c);

    vkCmdTraceRaysKHR(commandBuffer, &raygenRegion, &missRegion, &hitRegion,
                      &callableRegion, RAYTRACE_WIDTH, RAYTRACE_HEIGHT, 1);

    voxelWorld->updateVoxels(commandBuffer, vulkanContext, imageIndex);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
      throw std::runtime_error("failed to record raytracing command buffer!");
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
    renderPassInfo.renderPass = vulkanContext->getRenderPass();
    renderPassInfo.framebuffer =
        vulkanContext->getSwapChainFramebuffers()[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = vulkanContext->getSwapChainExtent();

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
    viewport.width = (float)vulkanContext->getSwapChainExtent().width;
    viewport.height = (float)vulkanContext->getSwapChainExtent().height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = vulkanContext->getSwapChainExtent();
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
    vkWaitForFences(vulkanContext->getDevice(), 1,
                    &syncManager->getInFlightFences()[currentFrame], VK_TRUE,
                    UINT64_MAX);
    camera->update(windowManager, voxelWorld, currentFrame);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(
        vulkanContext->getDevice(), vulkanContext->getSwapChain(), UINT64_MAX,
        syncManager->getImageAvailableSemaphores()[currentFrame],
        VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
      vulkanContext->recreateSwapchain(windowManager);
      return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
      throw std::runtime_error("failed to acquire swap chain image!");
    }

    vkResetFences(vulkanContext->getDevice(), 1,
                  &syncManager->getInFlightFences()[currentFrame]);

    vkResetCommandBuffer(commandManager->getCommandBuffers()[currentFrame],
                         /*VkCommandBufferResetFlagBits*/ 0);
    vkResetCommandBuffer(
        raytracingCommandBuffers[(currentFrame - 1 + MAX_FRAMES_IN_FLIGHT) %
                                 MAX_FRAMES_IN_FLIGHT],
        0);

    recordCommandBuffer(commandManager->getCommandBuffers()[currentFrame],
                        imageIndex);
    recordVoxelCommandBuffer(
        raytracingCommandBuffers[(currentFrame - 1 + MAX_FRAMES_IN_FLIGHT) %
                                 MAX_FRAMES_IN_FLIGHT],
        (currentFrame - 1 + MAX_FRAMES_IN_FLIGHT) % MAX_FRAMES_IN_FLIGHT,
        section);

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
        raytracingCommandBuffers[(currentFrame - 1 + MAX_FRAMES_IN_FLIGHT) %
                                 MAX_FRAMES_IN_FLIGHT]);
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

    VkSwapchainKHR swapChains[] = {vulkanContext->getSwapChain()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(vulkanContext->getPresentQueue(), &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
        windowManager->framebufferResized) {
      windowManager->framebufferResized = false;
      vulkanContext->recreateSwapchain(windowManager);
    } else if (result != VK_SUCCESS) {
      throw std::runtime_error("failed to present swap chain image!");
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    if (currentFrame == 0)
      section = (section + 1) % 16;
  }

  SwapChainSupportDetails VoxelEngine::querySwapChainSupport(VkPhysicalDevice device) {
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        device, vulkanContext->getSurface(), &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, vulkanContext->getSurface(),
                                         &formatCount, nullptr);

    if (formatCount != 0) {
      details.formats.resize(formatCount);
      vkGetPhysicalDeviceSurfaceFormatsKHR(device, vulkanContext->getSurface(),
                                           &formatCount,
                                           details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        device, vulkanContext->getSurface(), &presentModeCount, nullptr);

    if (presentModeCount != 0) {
      details.presentModes.resize(presentModeCount);
      vkGetPhysicalDeviceSurfacePresentModesKHR(
          device, vulkanContext->getSurface(), &presentModeCount,
          details.presentModes.data());
    }

    return details;
  }