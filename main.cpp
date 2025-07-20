#include "CommandManager.hpp"
#include "PipelineManager.hpp"
#include "Raytracer.hpp"
#include "ResourceManager.hpp"
#include "SyncManager.hpp"
#include "VoxelWorld.hpp"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "WindowManager.hpp"
#include "VulkanContext.hpp"
#include <memory>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <optional>
#include <set>
#include <thread>
#include <chrono>
#include <queue>
#include <unordered_set>
#include <mutex>
#include <condition_variable>
#include <random>

#define FNL_IMPL
#include "FastNoiseLite.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONCe
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

static float semiRandomFloat(float x, float y, float z);

const uint32_t WIDTH = 1920*4;
const uint32_t HEIGHT = 1280*4;

#define RAYTRACE_HEIGHT 1080
#define RAYTRACE_WIDTH 1920

class VoxelEngine {
public:
    void run() {


        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    std::unique_ptr<WindowManager> windowManager;
    std::unique_ptr<VulkanContext> vulkanContext;
    std::unique_ptr<SyncManager> syncManager;
    std::unique_ptr<CommandManager> commandManager;
    std::unique_ptr<PipelineManager> pipelineManager;
    std::unique_ptr<Raytracer> raytracer;
    std::unique_ptr<VoxelWorld> voxelWorld;

    std::vector<VkCommandBuffer> raytracingCommandBuffers;

    uint32_t currentFrame = 0;
    uint8_t section = 0;

    // Raytracin

    

    VkStridedDeviceAddressRegionKHR raygenRegion{};
    VkStridedDeviceAddressRegionKHR missRegion{};
    VkStridedDeviceAddressRegionKHR hitRegion{};
    VkStridedDeviceAddressRegionKHR callableRegion{};
    VkDeviceSize sbtSize;
    VkBuffer sbtBuffer;
    VkDeviceMemory sbtMemory;

    std::vector<int> frame;

    std::vector<VkBuffer> uniformBuffer;
    std::vector<VkDeviceMemory> uniformBufferMemory;
    TransformUBO ubo{};

    std::vector<void*> uniformBuffersMapped;

    // Camera

    glm::vec3 cameraTargetPoint = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 cameraPosition = glm::vec3(-512, -512, -512);//glm::vec3(-512.0f, -512.0f, -512.0f);
    glm::ivec3 chunkPosition = glm::ivec3(0,0,0);

    glm::vec3 cameraVelocity = glm::vec3(0.0f, 0.0f, 0.0f);

    float pitch = 0.0f;  // Rotation around the X-axis (up/down)
    float yaw = -90.0f;  // Rotation around the Y-axis (left/right)
    float lastMouseX = 0.0f;
    float lastMouseY = 0.0f;

    float mouseX = 0.0f, mouseY = 0.0f;

    float movementSpeed = 7.0f;
    float mouseSensitivity = 0.1f;

    bool framebufferResized = false;

    // Voxels

    

    // double deltaTime = 0;
    // double lastTime = 0;

    void initWindow() {
        frame.resize(MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            frame[i] = 0;
        }

        windowManager = std::make_unique<WindowManager>(WIDTH, HEIGHT, "Voxel Engine");
    }

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto app = reinterpret_cast<VoxelEngine*>(glfwGetWindowUserPointer(window));
        app->framebufferResized = true;
    }

    void initVulkan() {
        vulkanContext = std::make_unique<VulkanContext>();
        vulkanContext->init(windowManager);
        // createInstance();
        // printf("Created instance\n");
        // setupDebugMessenger();
        // printf("Setup debug messenger\n");
        // createSurface();
        // printf("Created surface\n");
        // pickPhysicalDevice();
        // printf("Picked physical device\n");
        // createLogicalDevice();
        // printf("Created logical device\n");
        // createSwapChain();
        // printf("Created swap chain\n");
        createTransformUBO();
        printf("Created transform UBO\n");
        // createImageViews();
        // printf("Created image views\n");
        // createRenderPass();
        // printf("Created graphics pipeline\n");
        // createFramebuffers();

        commandManager = std::make_unique<CommandManager>(vulkanContext);

        // printf("Created render pass\n");
        // createCommandPool();
        // printf("Created command pool\n");
        createCommandBuffers();
        printf("Created command buffers\n");

        // createVoxelResources();
        // printf("Created voxel resources\n");

        voxelWorld = std::make_unique<VoxelWorld>(vulkanContext, commandManager);

        raytracer = std::make_unique<Raytracer>(commandManager, vulkanContext, voxelWorld, uniformBuffer);

        

        // createRaytracingStorageImage();
        // printf("Created raytracing storage image\n");
        // createGraphicsPipeline();
        // printf("Created framebuffers\n");
        pipelineManager = std::make_unique<PipelineManager>(vulkanContext, raytracer);
        // createSyncObjects();
        // printf("Created sync objects\n");

        syncManager = std::make_unique<SyncManager>(vulkanContext);

        
        printf("Created raytracing pipeline\n");
        createRaytracingRegions();
        printf("Created raytracing regions\n");
    }

    void mainLoop() {
        while (!windowManager->shouldClose()) {
            windowManager->pollEvents();
            drawFrame();
        }

        vkDeviceWaitIdle(vulkanContext->getDevice());
    }

    void cleanup() {
        // cleanupSwapChain();

        // vkDestroyPipeline(device, graphicsPipeline, nullptr);
        // vkDestroyPipelineLayout(device, pipelineLayout, nullptr);

        // vkDestroyRenderPass(device, renderPass, nullptr);

        // for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        //     vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
        //     vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
        //     vkDestroyFence(device, inFlightFences[i], nullptr);
        // }

        // vkDestroyCommandPool(device, commandPool, nullptr);

        // vkDestroyDevice(device, nullptr);

        // if (enableValidationLayers) {
        //     DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        // }

        // vkDestroySurfaceKHR(instance, surface, nullptr);
        // vkDestroyInstance(instance, nullptr);
    }

    void recreateSwapChain() {
        windowManager->recreateWindow();

        vkDeviceWaitIdle(vulkanContext->getDevice());

        vulkanContext->recreateSwapchain(windowManager);
    }

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = /*VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |*/ VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
    }


#define ALIGN_UP(value, alignment) (((value) + (alignment) - 1) & ~((alignment) - 1))


    void createBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
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
        allocInfo.memoryTypeIndex = ResourceManager::findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate buffer memory!");
        }

        vkBindBufferMemory(device, buffer, bufferMemory, 0);
    }


    void createRaytracingRegions()
    {
        VkPhysicalDeviceProperties2 deviceProperties2 = {};
        deviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        VkPhysicalDeviceRayTracingPipelinePropertiesKHR raytracingPipelineProperties = {};
        raytracingPipelineProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
        deviceProperties2.pNext = &raytracingPipelineProperties;
        vkGetPhysicalDeviceProperties2(vulkanContext->getPhysicalDevice(), &deviceProperties2);

        VkDeviceSize handleSize = raytracingPipelineProperties.shaderGroupHandleSize;
        VkDeviceSize handleSizeAligned = ALIGN_UP(handleSize, raytracingPipelineProperties.shaderGroupBaseAlignment);
        sbtSize = handleSizeAligned * 2;

        createBuffer(vulkanContext->getDevice(), vulkanContext->getPhysicalDevice(), sbtSize, VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, sbtBuffer, sbtMemory);


        std::vector<uint8_t> shaderHandleStorage(sbtSize);

        PFN_vkGetRayTracingShaderGroupHandlesKHR vkGetRayTracingShaderGroupHandlesKHR = reinterpret_cast<PFN_vkGetRayTracingShaderGroupHandlesKHR>(vkGetDeviceProcAddr(vulkanContext->getDevice(), "vkGetRayTracingShaderGroupHandlesKHR"));
        vkGetRayTracingShaderGroupHandlesKHR(vulkanContext->getDevice(), raytracer->getPipeline(), 0, 2, sbtSize, shaderHandleStorage.data());

        void* mappedData;
        vkMapMemory(vulkanContext->getDevice(), sbtMemory, 0, sbtSize, 0, &mappedData);
        memcpy(mappedData, shaderHandleStorage.data(), sbtSize);
        vkUnmapMemory(vulkanContext->getDevice(), sbtMemory);


        VkBufferDeviceAddressInfo bufferAddressInfo = {};
        bufferAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
        bufferAddressInfo.buffer = sbtBuffer;
        VkDeviceAddress sbtAddress = vkGetBufferDeviceAddress(vulkanContext->getDevice(), &bufferAddressInfo);

        raygenRegion.deviceAddress = sbtAddress;
        raygenRegion.stride = handleSizeAligned;
        raygenRegion.size = handleSizeAligned;

        missRegion.deviceAddress = sbtAddress + handleSizeAligned;
        missRegion.stride = handleSizeAligned;
        missRegion.size = handleSizeAligned;

        hitRegion = {};
        callableRegion = {};   
    }

    static void TerrainWork(std::vector<uint8_t*> voxelData, FastNoiseLite* noise, int z0, int SLICE_THICKNESS)
    {
        // for (int z = z0; z < z0 + SLICE_THICKNESS; z++)
        // {
        //     for (int y = 0; y < TERRAIN_AREA; y++)
        //     {
        //         for (int x = 0; x < TERRAIN_AREA; x++)
        //         {
        //             uint8_t base = 0b11100000;
        //             float v = (noise->GetNoise(x * 0.1f * TERRAIN_SCALE, y * 0.1f * TERRAIN_SCALE, z * 0.4f * TERRAIN_SCALE) + noise->GetNoise(x * 1.5f * TERRAIN_SCALE, y * 1.5f * TERRAIN_SCALE, z * 1.0f * TERRAIN_SCALE) * 0.03) / 1.03f;

        //             int material = v <= 0.1;
        //             if (material != MAT_AIR)
        //             {
        //                 if (z > 0 && (voxel_mat(vox(x, y, z - 1)) == MAT_AIR))
        //                 {
        //                     material = MAT_GRASS;
        //                     float v = semiRandomFloat(x, y, z);
        //                     if (v < 0.08)
        //                     {
        //                         vox(x, y, z - 1) |= MAT_GRASS;

        //                         if (z > 1 && v < 0.008)
        //                         {
        //                             vox(x, y, z - 2) |= MAT_FLOWER;
        //                         }
        //                     }
        //                 }
        //                 else
        //                 {
        //                     float v = noise->GetNoise(x * 1.5f * TERRAIN_SCALE, y * 1.5f * TERRAIN_SCALE, z * 1.f * TERRAIN_SCALE);
        //                     int32_t i = *((int32_t*)&v);
        //                     int32_t i2 = i | 9;
        //                     float v2 = *((float*)(&i2));
        //                     if (v2 < -0.15)
        //                     {
        //                         material = (i & 15) < 2 ? MAT_STONE2 : MAT_STONE3;
        //                     }
        //                 }
        //             }

        //             vox(x, y, z) = base | material;
        //         }
        //     }
        // }


    }

    void createCommandBuffers() {
        raytracingCommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo raytracingAllocInfo{};
        raytracingAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        raytracingAllocInfo.commandPool = commandManager->getCommandPool();
        raytracingAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        raytracingAllocInfo.commandBufferCount = (uint32_t) commandManager->getCommandBuffers().size();

        if (vkAllocateCommandBuffers(vulkanContext->getDevice(), &raytracingAllocInfo, raytracingCommandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }

    struct PushConstant
    {
        uint32_t flag;
        uint32_t frame;
    };

    void recordVoxelCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, uint8_t section)
    {
  
        frame[imageIndex]++;
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to begin recording command buffer!");
        }
                

      
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, raytracer->getPipeline());

        vkCmdBindDescriptorSets(
            commandBuffer,
            VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
            raytracer->getPipelineLayout(),
            0,
            1,
            &raytracer->getDescriptorSet(currentFrame),
            0,
            nullptr
        );

        PFN_vkCmdTraceRaysKHR vkCmdTraceRaysKHR = reinterpret_cast<PFN_vkCmdTraceRaysKHR>(vkGetDeviceProcAddr(vulkanContext->getDevice(), "vkCmdTraceRaysKHR"));

        PushConstant c;
        c.flag = 0;
        c.frame = frame[imageIndex];
        vkCmdPushConstants(
            commandBuffer,
            raytracer->getPipelineLayout(),
            VK_SHADER_STAGE_RAYGEN_BIT_KHR,
            0,
            8,
            &c 
        );
        vkCmdTraceRaysKHR(
            commandBuffer,
            &raygenRegion,
            &missRegion,
            &hitRegion,
            &callableRegion,
            RAYTRACE_WIDTH,
            RAYTRACE_HEIGHT,
            1
        );

        VkMemoryBarrier barrier = {};
barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;  // Ensure writes from the first trace finish
barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;   // Ensure the second trace can read them
barrier.pNext = 0;

vkCmdPipelineBarrier(
    commandBuffer, 
    VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR, // Source: First trace rays execution
    VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR, // Destination: Second trace rays execution
    0, 
    1, &barrier, 
    0, nullptr, 
    0, nullptr
);

 c.flag = 1;
        vkCmdPushConstants(
            commandBuffer,
            raytracer->getPipelineLayout(),
            VK_SHADER_STAGE_RAYGEN_BIT_KHR,
            0,
            4,
            &c
        );

        vkCmdTraceRaysKHR(
            commandBuffer,
            &raygenRegion,
            &missRegion,
            &hitRegion,
            &callableRegion,
            RAYTRACE_WIDTH,
            RAYTRACE_HEIGHT,
            1
        );
vkCmdPipelineBarrier(
    commandBuffer, 
    VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR, // Source: First trace rays execution
    VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR, // Destination: Second trace rays execution
    0, 
    1, &barrier, 
    0, nullptr, 
    0, nullptr
);

 c.flag = 2;
        vkCmdPushConstants(
            commandBuffer,
            raytracer->getPipelineLayout(),
            VK_SHADER_STAGE_RAYGEN_BIT_KHR,
            0,
            4,
            &c
        );

        vkCmdTraceRaysKHR(
            commandBuffer,
            &raygenRegion,
            &missRegion,
            &hitRegion,
            &callableRegion,
            RAYTRACE_WIDTH,
            RAYTRACE_HEIGHT,
            1
        );


        voxelWorld->updateVoxels(commandBuffer, vulkanContext, imageIndex);
        

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to record raytracing command buffer!");
        }
    }

    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = vulkanContext->getRenderPass();
        renderPassInfo.framebuffer = vulkanContext->getSwapChainFramebuffers()[imageIndex];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = vulkanContext->getSwapChainExtent();

        VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineManager->getGraphicsPipeline());

            VkViewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = (float) vulkanContext->getSwapChainExtent().width;
            viewport.height = (float) vulkanContext->getSwapChainExtent().height;
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

            VkRect2D scissor{};
            scissor.offset = {0, 0};
            scissor.extent = vulkanContext->getSwapChainExtent();
            vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

            vkCmdBindDescriptorSets(
                commandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                pipelineManager->getGraphicsPipelineLayout(),
                0,
                1,
                &pipelineManager->getDescriptorSet(currentFrame),
                0,
                nullptr
            );

            vkCmdDraw(commandBuffer, 6, 1, 0, 0);

        vkCmdEndRenderPass(commandBuffer);

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }

    void drawFrame() {
        vkWaitForFences(vulkanContext->getDevice(), 1, &syncManager->getInFlightFences()[currentFrame], VK_TRUE, UINT64_MAX);
        UpdateUBO(currentFrame);
              memcpy(uniformBuffersMapped[currentFrame], &ubo, sizeof(TransformUBO));


        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(vulkanContext->getDevice(), vulkanContext->getSwapChain(), UINT64_MAX, syncManager->getImageAvailableSemaphores()[currentFrame], VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain();
            return;
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        vkResetFences(vulkanContext->getDevice(), 1, &syncManager->getInFlightFences()[currentFrame]);

        vkResetCommandBuffer(commandManager->getCommandBuffers()[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
        vkResetCommandBuffer(raytracingCommandBuffers[(currentFrame - 1 + MAX_FRAMES_IN_FLIGHT) % MAX_FRAMES_IN_FLIGHT], 0);

        recordCommandBuffer(commandManager->getCommandBuffers()[currentFrame], imageIndex);
        recordVoxelCommandBuffer(raytracingCommandBuffers[(currentFrame - 1 + MAX_FRAMES_IN_FLIGHT) % MAX_FRAMES_IN_FLIGHT], (currentFrame - 1 + MAX_FRAMES_IN_FLIGHT) % MAX_FRAMES_IN_FLIGHT, section);
        
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { syncManager->getImageAvailableSemaphores()[currentFrame] };
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;


        std::vector<VkCommandBuffer> commands;
        commands.push_back(raytracingCommandBuffers[(currentFrame - 1 + MAX_FRAMES_IN_FLIGHT) % MAX_FRAMES_IN_FLIGHT]);
        commands.push_back(commandManager->getCommandBuffers()[currentFrame]);

        submitInfo.commandBufferCount = 2;
        submitInfo.pCommandBuffers = commands.data();

        VkSemaphore signalSemaphores[] = { syncManager->getRenderFinishedSemaphores()[currentFrame]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(vulkanContext->getGraphicsQueue(), 1, &submitInfo, syncManager->getInFlightFences()[currentFrame]) != VK_SUCCESS) {
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

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
            framebufferResized = false;
            recreateSwapChain();
        } else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
        if (currentFrame == 0)
        section = (section + 1) % 16;
    }

    void createTransformUBO()
    {
        uniformBuffer.resize(MAX_FRAMES_IN_FLIGHT);
        uniformBufferMemory.resize(MAX_FRAMES_IN_FLIGHT);
        uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            VkDeviceSize bufferSize = sizeof(TransformUBO);

            VkBufferCreateInfo bufferInfo = {};
            bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferInfo.size = bufferSize;
            bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
            bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            if (vkCreateBuffer(vulkanContext->getDevice(), &bufferInfo, nullptr, &uniformBuffer[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create uniform buffer");
            }

            VkMemoryRequirements memRequirements;
            vkGetBufferMemoryRequirements(vulkanContext->getDevice(), uniformBuffer[i], &memRequirements);

            VkMemoryAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = memRequirements.size;
            allocInfo.memoryTypeIndex = ResourceManager::findMemoryType(vulkanContext->getPhysicalDevice(), memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

            if (vkAllocateMemory(vulkanContext->getDevice(), &allocInfo, nullptr, &uniformBufferMemory[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to allocate uniform buffer memory!");
            }

            vkBindBufferMemory(vulkanContext->getDevice(), uniformBuffer[i], uniformBufferMemory[i], 0);

    
            vkMapMemory(vulkanContext->getDevice(), uniformBufferMemory[i], 0, sizeof(TransformUBO), 0, &uniformBuffersMapped[i]);
            memcpy(uniformBuffersMapped[i], &ubo, sizeof(TransformUBO));
            ubo.view = glm::mat4(1.0);
            ubo.proj = glm::perspective(glm::radians(70.0f), vulkanContext->getSwapChainExtent().width / (float)vulkanContext->getSwapChainExtent().height, 0.1f, 1000.0f);
            ubo.proj[1][1] *= -1;
            ubo.proj = glm::inverse(ubo.proj);
        }
    }

    void onMouseMove(double xPos, double yPos) {
        mouseX = (float)xPos;
        mouseY = (float)yPos;
    }

    bool firstMouse = true;


    bool rayIntersectsCube(glm::vec3 corner, float size, glm::vec3 rayOrigin, glm::vec3 rayDir, float& tMin, float& tMax)
    {
        glm::vec3 sizeVec = glm::vec3{size};
        glm::vec3 minBound = corner;
        glm::vec3 maxBound = corner + sizeVec;

        glm::vec3 invDir = 1.0f / rayDir;
        glm::vec3 t1 = (minBound - rayOrigin) * invDir;
        glm::vec3 t2 = (maxBound - rayOrigin) * invDir;

        glm::vec3 tMinVec = glm::min(t1, t2);
        glm::vec3 tMaxVec = glm::max(t1, t2);

        tMin = std::max(std::max(tMinVec.x, tMinVec.y), tMinVec.z);
        tMax = std::min(std::min(tMaxVec.x, tMaxVec.y), tMaxVec.z);

        return tMax >= tMin && tMax >= 0;
    }

    glm::ivec3 rayCast(glm::vec3 rayOrigin, glm::vec3 rayDir, bool& something_hit, float maxDistance)
    {
        float tMax = 0, tMin = 0;

        glm::ivec3 origin = glm::ivec3(rayOrigin);

        uint8_t value = voxelWorld->getChunk(origin)[((int)origin.z % 128) * 128*128 + ((int)origin.y % 128) * 128 + ((int)origin.x % 128)];
        uint8_t layer = ((value >> 5) & 7);
        uint8_t material = value & 31;

        if (rayIntersectsCube((origin / (int)std::pow(2, layer)) * (int)std::pow(2, layer), std::pow(2, layer), rayOrigin, rayDir, tMin, tMax))
        {
            glm::vec3 hit = rayOrigin;
            float distanceToBox = tMin;
            float rayDistance = distanceToBox;

            distanceToBox = std::abs(tMax);
            hit += rayDir * distanceToBox;
            hit += rayDir * 0.001f;

            value = voxelWorld->getChunk(hit)[((int)hit.z % 128) * 128*128 + ((int)hit.y % 128) * 128 + ((int)hit.x % 128)];
            layer = ((value >> 5) & 7);
            material = value & 31;

            if (material != MAT_AIR)
            {
                printf("HIT: %i\n", material);
                something_hit = true;
                return hit;
            }

            for (int i = 0; i < 200; i++)
            {
                rayIntersectsCube((glm::ivec3(hit) / (int)std::pow(2, layer)) * (int)std::pow(2, layer), std::pow(2, layer), rayOrigin, rayDir, tMin, tMax);
                distanceToBox = std::abs(tMin - tMax);
                hit += rayDir * distanceToBox;
                rayDistance += distanceToBox;

                value = voxelWorld->getChunk(hit)[((int)hit.z % 128) * 128*128 + ((int)hit.y % 128) * 128 + ((int)hit.x % 128)];
                layer = ((value >> 5) & 7);
                material = value & 31;

                if (material != MAT_AIR)
                {
                    printf("HIT: %i\n", material);
                    something_hit = true;
                    return hit;
                }

                if (rayDistance > maxDistance) break;
            }
            something_hit = false;
            return hit;
        }
        something_hit = false;
        return glm::vec3(0);
    }

    void UpdateUBO(int currentFrame)
    {

          struct {
            float& pos;
            int lowerBound, upperBound, offset, modValue, shift;
        } checks[] = {
            {cameraPosition.x, -640, -340, 128, 8, 1},
            {cameraPosition.y, -640, -340, 128, 64, 8},
            {cameraPosition.z, -640, -340, 128, 512, 64}
        };
        
        cameraPosition.z += cameraVelocity.z * windowManager->getDeltaTime();
        cameraTargetPoint.z += cameraVelocity.z * windowManager->getDeltaTime();;

        glm::ivec3 intCameraPosition = glm::ivec3(cameraPosition) * -1;

        #define voxChunk(chunk, x,y,z) chunk[((z) % 128) * 128 * 128 + ((y) % 128) * 128 + ((x) % 128)]
        // floor
        bool is_grounded = false;
        for (int x = -1; x < 2; x++)
        {
            for (int y = -1; y < 2; y++)
            {
                uint16_t chunkID = ((intCameraPosition.z + 15) / 128) * 64 + ((intCameraPosition.y + y) / 128) * 8 + ((intCameraPosition.x + x) / 128);
                is_grounded |= (MAT_HAS_COLLISION(voxel_mat(voxChunk(voxelWorld->voxelData[voxelWorld->voxelChunkMapData[chunkID]].data, intCameraPosition.x + x, intCameraPosition.y + y, intCameraPosition.z + 15)))); //MAT_HAS_COLLISION(voxel_mat(vox(intCameraPosition.x + x, intCameraPosition.y + y, intCameraPosition.z + 30)));
            }
        }
        printf("Got here %d\n", 2);
        // // ceiling
        bool is_ouch = false;
        // for (int x = -1; x < 2; x++)
        // {
        //     for (int y = -1; y < 2; y++)
        //     {
        //         is_ouch |= MAT_HAS_COLLISION(voxel_mat(vox(intCameraPosition.x + x, intCameraPosition.y + y, intCameraPosition.z - 6)));
        //     }
        // }
     
        if (is_grounded)
        {
            cameraVelocity.z = 0;
        }
        else if (is_ouch)
        {
            cameraVelocity.z = -fabsf(cameraVelocity.z) * 0.8;
        }
        else
        {
            cameraVelocity.z -= 20 * 7 * windowManager->getDeltaTime();;
        }

        if (windowManager->isMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT)) {
            // Hide the cursor and capture mouse movement

            
            windowManager->hideCursor();

            float deltaTime = 0.1f;

            movementSpeed = windowManager->isKeyPressed(GLFW_KEY_LEFT_SHIFT) ? 11.0 : 7.0;
            bool reset = false;
            if (windowManager->isKeyPressed(GLFW_KEY_S)) { reset = true; cameraPosition += movementSpeed * deltaTime * glm::normalize(cameraTargetPoint - cameraPosition) * glm::vec3(1, 1, 0); }
            if (windowManager->isKeyPressed(GLFW_KEY_W)) { reset = true; cameraPosition -= movementSpeed * deltaTime * glm::normalize(cameraTargetPoint - cameraPosition) * glm::vec3(1, 1, 0); }
            if (windowManager->isKeyPressed(GLFW_KEY_D)) { reset = true; cameraPosition -= glm::normalize(glm::cross(cameraTargetPoint - cameraPosition, glm::vec3(0.0f, 0.0f, 1.0f))) * movementSpeed * deltaTime * glm::vec3(1, 1, 0); }
            if (windowManager->isKeyPressed(GLFW_KEY_A)) { reset = true; cameraPosition += glm::normalize(glm::cross(cameraTargetPoint - cameraPosition, glm::vec3(0.0f, 0.0f, 1.0f))) * movementSpeed * deltaTime * glm::vec3(1, 1, 0); }

            // Q and E for vertical movement
            if (windowManager->isKeyPressed(GLFW_KEY_Q)) { cameraPosition.z -= movementSpeed * 2 * deltaTime; }
            if ((windowManager->isKeyPressed(GLFW_KEY_SPACE) || windowManager->isKeyPressed(GLFW_KEY_E)) && is_grounded)
            {
                cameraVelocity.z += 600 * deltaTime;
            }

            if (cameraVelocity.length() != 0 || !is_grounded)
            {
                reset = true;
            }

            double currentMouseX, currentMouseY;
            windowManager->getCursorPos(&currentMouseX, &currentMouseY);

            if (firstMouse) {
                lastMouseX = static_cast<float>(currentMouseX);
                lastMouseY = static_cast<float>(currentMouseY);
                firstMouse = false;
            }

            // Calculate the offset
            float offsetX = static_cast<float>(currentMouseX - lastMouseX);
            float offsetY = static_cast<float>(lastMouseY - currentMouseY);  // Reverse Y for correct orientation
            lastMouseX = static_cast<float>(currentMouseX);
            lastMouseY = static_cast<float>(currentMouseY);

            // Apply sensitivity
            offsetX *= mouseSensitivity;
            offsetY *= mouseSensitivity;

            yaw -= offsetX;
            pitch -= offsetY;

               if (reset || offsetX != 0 || offsetY != 0)
                {
                    for (int i = 0; i < frame.size(); i++)
                    {
                        frame[i] = 0;
                    }
                }


            // Constrain pitch to prevent flipping
            if (pitch > 89.0f) pitch = 89.0f;
            if (pitch < -89.0f) pitch = -89.0f;

            // Update the camera target based on yaw and pitch
            glm::vec3 direction;
            direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
            direction.z = sin(glm::radians(pitch));
            direction.y = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
            direction = glm::normalize(direction);

            cameraTargetPoint = cameraPosition + direction;
        } else {
            // Release the cursor and reset first mouse flag
            windowManager->showCursor();
            firstMouse = true;
        }




        ubo.view = glm::inverse(glm::lookAt(cameraPosition, cameraTargetPoint, glm::vec3(0.0f, 0.0f, 1.0f)));

        printf("\rPOSITION: %i %i %i\tCHUNK: %i %i %i\tDELTA TIME: %f\to: %i", (int)cameraPosition.x, (int)cameraPosition.y, (int)cameraPosition.z, chunkPosition.x, chunkPosition.y, chunkPosition.z, windowManager->getDeltaTime(), is_ouch);


        

        for (auto& check : checks) {
            if (check.pos < check.lowerBound) {
                check.pos += check.offset;
                voxelWorld->updateVoxelChunkMap(check.modValue, check.shift);
            } else if (check.pos > check.upperBound) {
                check.pos -= check.offset;
                voxelWorld->updateVoxelChunkMap(check.modValue, check.modValue - check.shift);
            }
        }

        voxelWorld->sortChunks();

        // Raycast and print distance
        if (windowManager->isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT))
        {
            bool hit = false;
            glm::ivec3 hitPosition = rayCast(-cameraPosition, cameraTargetPoint - cameraPosition, hit, 25.0f);
            if (hit)
            {   
                printf("HIT: %i, %i %i %i\n", hit, hitPosition.x, hitPosition.y, hitPosition.z);
                voxelWorld->getChunk(hitPosition)[((int)hitPosition.z % 128) * 128*128 + ((int)hitPosition.y % 128) * 128 + ((int)hitPosition.x % 128)] = 0;

                for (int x = -4; x <= 4; x++)
                {
                    for (int y = -4; y <= 4; y++)
                    {
                        for (int z = -4; z <= 4; z++)
                        {
                            glm::ivec3 offset = glm::ivec3(x, y, z);
                            if (voxel_mat(voxelWorld->getChunk(hitPosition + offset)[(((int)hitPosition.z + z) % 128) * 128*128 + (((int)hitPosition.y + y) % 128) * 128 + (((int)hitPosition.x + x) % 128)]) != MAT_AIR)
                            voxelWorld->getChunk(hitPosition + offset)[(((int)hitPosition.z + z) % 128) * 128*128 + (((int)hitPosition.y + y) % 128) * 128 + (((int)hitPosition.x + x) % 128)] = 0;
                        }
                    }
                }
                
                uint16_t chunkID = voxelWorld->getChunkID(hitPosition);
                
                voxelWorld->chunkUpdateQueue[++voxelWorld->chunkUpdateQueue[0]] = chunkID;
            }
        }

    }

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
        SwapChainSupportDetails details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, vulkanContext->getSurface(), &details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, vulkanContext->getSurface(), &formatCount, nullptr);

        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, vulkanContext->getSurface(), &formatCount, details.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, vulkanContext->getSurface(), &presentModeCount, nullptr);

        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, vulkanContext->getSurface(), &presentModeCount, details.presentModes.data());
        }

        return details;
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }
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

int main() {
    VoxelEngine app;
    
    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}