#include "CommandManager.hpp"
#include "SyncManager.hpp"
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

struct TransformUBO
{
    glm::mat4 view;
    glm::mat4 proj;
};

#define RAYTRACE_HEIGHT 1080
#define RAYTRACE_WIDTH 1920



//=================
//  Materials
//-----------------
#define MAT_AIR 0
#define MAT_STONE 1
#define MAT_STONE2 2
#define MAT_STONE3 3
#define MAT_GRASS 9
#define MAT_FLOWER 10
#define MAT_WATER 19
#define MAT_PLANT_CORE 20
// ================
#include <iomanip>
#define voxel_mat(voxel) ((voxel) & 0b11111)
#define voxel_base(voxel) (((voxel) & 0b11100000) >> 5)
#define MAT_IS_STONE(mat) ((mat) >= MAT_STONE && (mat) <= MAT_STONE3)
#define MAT_HAS_COLLISION(mat) ((mat) > MAT_AIR && (mat) < MAT_FLOWER)

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

    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    std::vector<VkCommandBuffer> raytracingCommandBuffers;

    uint32_t currentFrame = 0;
    uint8_t section = 0;

    // Raytracing
    VkPipeline raytracingPipeline;
    VkPipelineLayout raytracingPipelineLayout;
    VkDescriptorSetLayout raytracingDescriptorSetLayout;
    VkDescriptorPool raytracingDescriptorPool;

    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool descriptorPool;

    std::vector<VkDescriptorSet> raytracingDescriptorSets;
    std::vector<VkDescriptorSet> descriptorSets;

    VkStridedDeviceAddressRegionKHR raygenRegion{};
    VkStridedDeviceAddressRegionKHR missRegion{};
    VkStridedDeviceAddressRegionKHR hitRegion{};
    VkStridedDeviceAddressRegionKHR callableRegion{};
    VkDeviceSize sbtSize;
    VkBuffer sbtBuffer;
    VkDeviceMemory sbtMemory;

    std::vector<int> frame;

    std::vector<VkImage> raytracingStorageImage;
    std::vector<VkImageView> raytracingStorageImageView;

    std::vector<VkImage> raytracingPositionStorageImage;
    std::vector<VkImageView> raytracingPositionStorageImageView;

    std::vector<VkImage> raytracingLightStorageImageX;
    std::vector<VkImageView> raytracingLightStorageImageViewX;

    std::vector<VkImage> raytracingLightStorageImageY;
    std::vector<VkImageView> raytracingLightStorageImageViewY;

    std::vector<VkImage> raytracingLightStorageImageZ;
    std::vector<VkImageView> raytracingLightStorageImageViewZ;

    std::vector<VkImage> raytracingLightStorageImageW;
    std::vector<VkImageView> raytracingLightStorageImageViewW;

    std::vector<VkSampler> imageSampler;

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

    std::vector<VkImage> voxelTexture;
    std::vector<VkDeviceMemory> voxelTexturesMemory;
    std::vector<VkImageView> voxelImageView;

    struct VoxelChunk
    {
    public:
        uint8_t* data;
        glm::ivec3 position;
        bool inQueue = false;
    };

    std::vector<VoxelChunk> voxelData;

    VkSampler voxelTextureSampler;

    std::vector<VkBuffer> voxelStagingBuffer;
    std::vector<VkDeviceMemory> voxelStagingBufferMemory;


    std::vector<VkImage> voxelChunkMapTexture;
    std::vector<VkDeviceMemory> voxelChunkMapTexturesMemory;
    std::vector<VkImageView> voxelChunkMapImageView;

    std::vector<VkBuffer> voxelChunkMapStagingBuffer;
    std::vector<VkDeviceMemory> voxelChunkMapStagingBufferMemory;

    uint16_t* voxelChunkMapData;

    FastNoiseLite noise;

    // double deltaTime = 0;
    // double lastTime = 0;

    void initWindow() {
        frame.resize(MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            frame[i] = 0;
        }
        noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);

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
        createRaytracingStorageImage();
        printf("Created raytracing storage image\n");
        createGraphicsPipeline();
        printf("Created framebuffers\n");
        // createSyncObjects();
        // printf("Created sync objects\n");

        syncManager = std::make_unique<SyncManager>(vulkanContext);

        createVoxelResources();
        printf("Created voxel resources\n");
        createRaytracingPipeline();
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

        createRaytracingStorageImage();
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
        allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);

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
        vkGetRayTracingShaderGroupHandlesKHR(vulkanContext->getDevice(), raytracingPipeline, 0, 2, sbtSize, shaderHandleStorage.data());

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

    void createRaytracingStorageImage()
    {
        raytracingStorageImage.resize(MAX_FRAMES_IN_FLIGHT);
        raytracingStorageImageView.resize(MAX_FRAMES_IN_FLIGHT);

        raytracingPositionStorageImage.resize(MAX_FRAMES_IN_FLIGHT);
        raytracingPositionStorageImageView.resize(MAX_FRAMES_IN_FLIGHT);

        raytracingLightStorageImageX.resize(MAX_FRAMES_IN_FLIGHT);
        raytracingLightStorageImageViewX.resize(MAX_FRAMES_IN_FLIGHT);

        raytracingLightStorageImageY.resize(MAX_FRAMES_IN_FLIGHT);
        raytracingLightStorageImageViewY.resize(MAX_FRAMES_IN_FLIGHT);

        raytracingLightStorageImageZ.resize(MAX_FRAMES_IN_FLIGHT);
        raytracingLightStorageImageViewZ.resize(MAX_FRAMES_IN_FLIGHT);

        raytracingLightStorageImageW.resize(MAX_FRAMES_IN_FLIGHT);
        raytracingLightStorageImageViewW.resize(MAX_FRAMES_IN_FLIGHT);
        
        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            {
                VkImageCreateInfo imageCreateInfo = {};
                imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
                imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
                imageCreateInfo.format = VK_FORMAT_R16G16B16A16_UNORM;
                imageCreateInfo.extent.width = RAYTRACE_WIDTH;
                imageCreateInfo.extent.height = RAYTRACE_HEIGHT;
                imageCreateInfo.extent.depth = 1;
                imageCreateInfo.mipLevels = 1;
                imageCreateInfo.arrayLayers = 1;
                imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
                imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
                imageCreateInfo.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
                imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
                imageCreateInfo.queueFamilyIndexCount = 0;
                imageCreateInfo.pQueueFamilyIndices = nullptr;
                imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

                if (vkCreateImage(vulkanContext->getDevice(), &imageCreateInfo, nullptr, &raytracingStorageImage[i]) != VK_SUCCESS)
                {
                    throw std::runtime_error("failed to create raytracing storage image!");
                }

                VkMemoryRequirements memRequirements;
                vkGetImageMemoryRequirements(vulkanContext->getDevice(), raytracingStorageImage[i], &memRequirements);

                VkMemoryAllocateInfo allocInfo = {};
                allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                allocInfo.allocationSize = memRequirements.size;

                VkPhysicalDeviceMemoryProperties memProperties;
                vkGetPhysicalDeviceMemoryProperties(vulkanContext->getPhysicalDevice(), &memProperties);
                for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
                {
                    if ((memRequirements.memoryTypeBits & (1 << i)) &&
                    (memProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
                    {
                        allocInfo.memoryTypeIndex = 1;
                        break;
                    }
                }

                VkDeviceMemory imageMemory;
                if (vkAllocateMemory(vulkanContext->getDevice(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
                {
                    throw std::runtime_error("Failed to allocate memory for image!");
                }

                vkBindImageMemory(vulkanContext->getDevice(), raytracingStorageImage[i], imageMemory, 0);

                // Create Image View


                VkImageViewCreateInfo viewCreateInfo = {};
                viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                viewCreateInfo.image = raytracingStorageImage[i];
                viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
                viewCreateInfo.format = VK_FORMAT_R16G16B16A16_UNORM;
                viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                viewCreateInfo.subresourceRange.baseMipLevel = 0;
                viewCreateInfo.subresourceRange.levelCount = 1;
                viewCreateInfo.subresourceRange.baseArrayLayer = 0;
                viewCreateInfo.subresourceRange.layerCount = 1;

                if (vkCreateImageView(vulkanContext->getDevice(), &viewCreateInfo, nullptr, &raytracingStorageImageView[i]) != VK_SUCCESS)
                {
                    throw std::runtime_error("failed to creaet raytracing image view!");
                }
                
                transitionImageLayout(raytracingStorageImage[i], VK_FORMAT_R16G16B16A16_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, 1);
            }
            {
                VkImageCreateInfo imageCreateInfo = {};
                imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
                imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
                imageCreateInfo.format = VK_FORMAT_R32_UINT;
                imageCreateInfo.extent.width = RAYTRACE_WIDTH;
                imageCreateInfo.extent.height = RAYTRACE_HEIGHT;
                imageCreateInfo.extent.depth = 1;
                imageCreateInfo.mipLevels = 1;
                imageCreateInfo.arrayLayers = 1;
                imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
                imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
                imageCreateInfo.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
                imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
                imageCreateInfo.queueFamilyIndexCount = 0;
                imageCreateInfo.pQueueFamilyIndices = nullptr;
                imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

                if (vkCreateImage(vulkanContext->getDevice(), &imageCreateInfo, nullptr, &raytracingPositionStorageImage[i]) != VK_SUCCESS)
                {
                    throw std::runtime_error("failed to create raytracing storage image!");
                }

                VkMemoryRequirements memRequirements;
                vkGetImageMemoryRequirements(vulkanContext->getDevice(), raytracingPositionStorageImage[i], &memRequirements);

                VkMemoryAllocateInfo allocInfo = {};
                allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                allocInfo.allocationSize = memRequirements.size;

                VkPhysicalDeviceMemoryProperties memProperties;
                vkGetPhysicalDeviceMemoryProperties(vulkanContext->getPhysicalDevice(), &memProperties);

                for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
                {
                    if ((memRequirements.memoryTypeBits & (1 << i)) &&
                    (memProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
                    {
                        allocInfo.memoryTypeIndex = 1;
                        break;
                    }
                }

                VkDeviceMemory imageMemory;
                if (vkAllocateMemory(vulkanContext->getDevice(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
                {
                    throw std::runtime_error("Failed to allocate memory for image!");
                }

                vkBindImageMemory(vulkanContext->getDevice(), raytracingPositionStorageImage[i], imageMemory, 0);

                // Create Image View


                VkImageViewCreateInfo viewCreateInfo = {};
                viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                viewCreateInfo.image = raytracingPositionStorageImage[i];
                viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
                viewCreateInfo.format = VK_FORMAT_R32_UINT;
                viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                viewCreateInfo.subresourceRange.baseMipLevel = 0;
                viewCreateInfo.subresourceRange.levelCount = 1;
                viewCreateInfo.subresourceRange.baseArrayLayer = 0;
                viewCreateInfo.subresourceRange.layerCount = 1;

                if (vkCreateImageView(vulkanContext->getDevice(), &viewCreateInfo, nullptr, &raytracingPositionStorageImageView[i]) != VK_SUCCESS)
                {
                    throw std::runtime_error("failed to creaet raytracing image view!");
                }

                transitionImageLayout(raytracingPositionStorageImage[i], VK_FORMAT_R32_UINT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, 1);
            }
            {
                VkImageCreateInfo imageCreateInfo = {};
                imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
                imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
                imageCreateInfo.format = VK_FORMAT_R32_UINT;
                imageCreateInfo.extent.width = RAYTRACE_WIDTH;
                imageCreateInfo.extent.height = RAYTRACE_HEIGHT;
                imageCreateInfo.extent.depth = 1;
                imageCreateInfo.mipLevels = 1;
                imageCreateInfo.arrayLayers = 1;
                imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
                imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
                imageCreateInfo.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
                imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
                imageCreateInfo.queueFamilyIndexCount = 0;
                imageCreateInfo.pQueueFamilyIndices = nullptr;
                imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

                if (vkCreateImage(vulkanContext->getDevice(), &imageCreateInfo, nullptr, &raytracingLightStorageImageX[i]) != VK_SUCCESS)
                {
                    throw std::runtime_error("failed to create raytracing storage image!");
                }

                VkMemoryRequirements memRequirements;
                vkGetImageMemoryRequirements(vulkanContext->getDevice(), raytracingLightStorageImageX[i], &memRequirements);

                VkMemoryAllocateInfo allocInfo = {};
                allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                allocInfo.allocationSize = memRequirements.size;

                VkPhysicalDeviceMemoryProperties memProperties;
                vkGetPhysicalDeviceMemoryProperties(vulkanContext->getPhysicalDevice(), &memProperties);

                for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
                {
                    if ((memRequirements.memoryTypeBits & (1 << i)) &&
                    (memProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
                    {
                        allocInfo.memoryTypeIndex = 1;
                        break;
                    }
                }

                VkDeviceMemory imageMemory;
                if (vkAllocateMemory(vulkanContext->getDevice(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
                {
                    throw std::runtime_error("Failed to allocate memory for image!");
                }

                vkBindImageMemory(vulkanContext->getDevice(), raytracingLightStorageImageX[i], imageMemory, 0);

                // Create Image View


                VkImageViewCreateInfo viewCreateInfo = {};
                viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                viewCreateInfo.image = raytracingLightStorageImageX[i];
                viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
                viewCreateInfo.format = VK_FORMAT_R32_UINT;
                viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                viewCreateInfo.subresourceRange.baseMipLevel = 0;
                viewCreateInfo.subresourceRange.levelCount = 1;
                viewCreateInfo.subresourceRange.baseArrayLayer = 0;
                viewCreateInfo.subresourceRange.layerCount = 1;

                if (vkCreateImageView(vulkanContext->getDevice(), &viewCreateInfo, nullptr, &raytracingLightStorageImageViewX[i]) != VK_SUCCESS)
                {
                    throw std::runtime_error("failed to creaet raytracing image view!");
                }

                transitionImageLayout(raytracingLightStorageImageX[i], VK_FORMAT_R32_UINT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, 1);
            }
            {
                VkImageCreateInfo imageCreateInfo = {};
                imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
                imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
                imageCreateInfo.format = VK_FORMAT_R32_UINT;
                imageCreateInfo.extent.width = RAYTRACE_WIDTH;
                imageCreateInfo.extent.height = RAYTRACE_HEIGHT;
                imageCreateInfo.extent.depth = 1;
                imageCreateInfo.mipLevels = 1;
                imageCreateInfo.arrayLayers = 1;
                imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
                imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
                imageCreateInfo.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
                imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
                imageCreateInfo.queueFamilyIndexCount = 0;
                imageCreateInfo.pQueueFamilyIndices = nullptr;
                imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

                if (vkCreateImage(vulkanContext->getDevice(), &imageCreateInfo, nullptr, &raytracingLightStorageImageY[i]) != VK_SUCCESS)
                {
                    throw std::runtime_error("failed to create raytracing storage image!");
                }

                VkMemoryRequirements memRequirements;
                vkGetImageMemoryRequirements(vulkanContext->getDevice(), raytracingLightStorageImageY[i], &memRequirements);

                VkMemoryAllocateInfo allocInfo = {};
                allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                allocInfo.allocationSize = memRequirements.size;

                VkPhysicalDeviceMemoryProperties memProperties;
                vkGetPhysicalDeviceMemoryProperties(vulkanContext->getPhysicalDevice(), &memProperties);

                for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
                {
                    if ((memRequirements.memoryTypeBits & (1 << i)) &&
                    (memProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
                    {
                        allocInfo.memoryTypeIndex = 1;
                        break;
                    }
                }

                VkDeviceMemory imageMemory;
                if (vkAllocateMemory(vulkanContext->getDevice(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
                {
                    throw std::runtime_error("Failed to allocate memory for image!");
                }

                vkBindImageMemory(vulkanContext->getDevice(), raytracingLightStorageImageY[i], imageMemory, 0);

                // Create Image View


                VkImageViewCreateInfo viewCreateInfo = {};
                viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                viewCreateInfo.image = raytracingLightStorageImageY[i];
                viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
                viewCreateInfo.format = VK_FORMAT_R32_UINT;
                viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                viewCreateInfo.subresourceRange.baseMipLevel = 0;
                viewCreateInfo.subresourceRange.levelCount = 1;
                viewCreateInfo.subresourceRange.baseArrayLayer = 0;
                viewCreateInfo.subresourceRange.layerCount = 1;

                if (vkCreateImageView(vulkanContext->getDevice(), &viewCreateInfo, nullptr, &raytracingLightStorageImageViewY[i]) != VK_SUCCESS)
                {
                    throw std::runtime_error("failed to creaet raytracing image view!");
                }

                transitionImageLayout(raytracingLightStorageImageY[i], VK_FORMAT_R32_UINT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, 1);
            }
            {
                VkImageCreateInfo imageCreateInfo = {};
                imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
                imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
                imageCreateInfo.format = VK_FORMAT_R32_UINT;
                imageCreateInfo.extent.width = RAYTRACE_WIDTH;
                imageCreateInfo.extent.height = RAYTRACE_HEIGHT;
                imageCreateInfo.extent.depth = 1;
                imageCreateInfo.mipLevels = 1;
                imageCreateInfo.arrayLayers = 1;
                imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
                imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
                imageCreateInfo.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
                imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
                imageCreateInfo.queueFamilyIndexCount = 0;
                imageCreateInfo.pQueueFamilyIndices = nullptr;
                imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

                if (vkCreateImage(vulkanContext->getDevice(), &imageCreateInfo, nullptr, &raytracingLightStorageImageZ[i]) != VK_SUCCESS)
                {
                    throw std::runtime_error("failed to create raytracing storage image!");
                }

                VkMemoryRequirements memRequirements;
                vkGetImageMemoryRequirements(vulkanContext->getDevice(), raytracingLightStorageImageZ[i], &memRequirements);

                VkMemoryAllocateInfo allocInfo = {};
                allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                allocInfo.allocationSize = memRequirements.size;

                VkPhysicalDeviceMemoryProperties memProperties;
                vkGetPhysicalDeviceMemoryProperties(vulkanContext->getPhysicalDevice(), &memProperties);

                for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
                {
                    if ((memRequirements.memoryTypeBits & (1 << i)) &&
                    (memProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
                    {
                        allocInfo.memoryTypeIndex = 1;
                        break;
                    }
                }

                VkDeviceMemory imageMemory;
                if (vkAllocateMemory(vulkanContext->getDevice(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
                {
                    throw std::runtime_error("Failed to allocate memory for image!");
                }

                vkBindImageMemory(vulkanContext->getDevice(), raytracingLightStorageImageZ[i], imageMemory, 0);

                // Create Image View


                VkImageViewCreateInfo viewCreateInfo = {};
                viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                viewCreateInfo.image = raytracingLightStorageImageZ[i];
                viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
                viewCreateInfo.format = VK_FORMAT_R32_UINT;
                viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                viewCreateInfo.subresourceRange.baseMipLevel = 0;
                viewCreateInfo.subresourceRange.levelCount = 1;
                viewCreateInfo.subresourceRange.baseArrayLayer = 0;
                viewCreateInfo.subresourceRange.layerCount = 1;

                if (vkCreateImageView(vulkanContext->getDevice(), &viewCreateInfo, nullptr, &raytracingLightStorageImageViewZ[i]) != VK_SUCCESS)
                {
                    throw std::runtime_error("failed to creaet raytracing image view!");
                }

                transitionImageLayout(raytracingLightStorageImageZ[i], VK_FORMAT_R32_UINT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, 1);
            }
            {
                VkImageCreateInfo imageCreateInfo = {};
                imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
                imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
                imageCreateInfo.format = VK_FORMAT_R32_UINT;
                imageCreateInfo.extent.width = RAYTRACE_WIDTH;
                imageCreateInfo.extent.height = RAYTRACE_HEIGHT;
                imageCreateInfo.extent.depth = 1;
                imageCreateInfo.mipLevels = 1;
                imageCreateInfo.arrayLayers = 1;
                imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
                imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
                imageCreateInfo.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
                imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
                imageCreateInfo.queueFamilyIndexCount = 0;
                imageCreateInfo.pQueueFamilyIndices = nullptr;
                imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

                if (vkCreateImage(vulkanContext->getDevice(), &imageCreateInfo, nullptr, &raytracingLightStorageImageW[i]) != VK_SUCCESS)
                {
                    throw std::runtime_error("failed to create raytracing storage image!");
                }

                VkMemoryRequirements memRequirements;
                vkGetImageMemoryRequirements(vulkanContext->getDevice(), raytracingLightStorageImageW[i], &memRequirements);

                VkMemoryAllocateInfo allocInfo = {};
                allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                allocInfo.allocationSize = memRequirements.size;

                VkPhysicalDeviceMemoryProperties memProperties;
                vkGetPhysicalDeviceMemoryProperties(vulkanContext->getPhysicalDevice(), &memProperties);

                for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
                {
                    if ((memRequirements.memoryTypeBits & (1 << i)) &&
                    (memProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
                    {
                        allocInfo.memoryTypeIndex = 1;
                        break;
                    }
                }

                VkDeviceMemory imageMemory;
                if (vkAllocateMemory(vulkanContext->getDevice(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
                {
                    throw std::runtime_error("Failed to allocate memory for image!");
                }

                vkBindImageMemory(vulkanContext->getDevice(), raytracingLightStorageImageW[i], imageMemory, 0);

                // Create Image View


                VkImageViewCreateInfo viewCreateInfo = {};
                viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                viewCreateInfo.image = raytracingLightStorageImageW[i];
                viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
                viewCreateInfo.format = VK_FORMAT_R32_UINT;
                viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                viewCreateInfo.subresourceRange.baseMipLevel = 0;
                viewCreateInfo.subresourceRange.levelCount = 1;
                viewCreateInfo.subresourceRange.baseArrayLayer = 0;
                viewCreateInfo.subresourceRange.layerCount = 1;

                if (vkCreateImageView(vulkanContext->getDevice(), &viewCreateInfo, nullptr, &raytracingLightStorageImageViewW[i]) != VK_SUCCESS)
                {
                    throw std::runtime_error("failed to creaet raytracing image view!");
                }

                transitionImageLayout(raytracingLightStorageImageW[i], VK_FORMAT_R32_UINT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, 1);
            }
        }
        
    }

    void createVoxelResources()
    {
        voxelTexture.resize(512);
        voxelImageView.resize(512);
        voxelTexturesMemory.resize(512);
        voxelStagingBuffer.resize(512);
        voxelStagingBufferMemory.resize(512);
        for (int i = 0; i < 512; i++)
        {
            VkImageCreateInfo imageInfo = {};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_3D;
            imageInfo.extent.width = 128;
            imageInfo.extent.height = 128;
            imageInfo.extent.depth = 128;
            imageInfo.mipLevels = 1;
            imageInfo.arrayLayers = 1;
            imageInfo.format = VK_FORMAT_R8_UINT;
            imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

            if (vkCreateImage(vulkanContext->getDevice(), &imageInfo, nullptr, &voxelTexture[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create texture image!");
            }

            // Allocate memory for the image
            VkMemoryRequirements memRequirements;
            vkGetImageMemoryRequirements(vulkanContext->getDevice(), voxelTexture[i], &memRequirements);

            VkMemoryAllocateInfo allocInfo = {};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = memRequirements.size;
            allocInfo.memoryTypeIndex = findMemoryType(vulkanContext->getPhysicalDevice(), memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

            if (vkAllocateMemory(vulkanContext->getDevice(), &allocInfo, nullptr, &voxelTexturesMemory[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to allocate memory for texture image!");
            }

            vkBindImageMemory(vulkanContext->getDevice(), voxelTexture[i], voxelTexturesMemory[i], 0);

            VkImageViewCreateInfo viewInfo = {};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = voxelTexture[i];
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
            viewInfo.format = VK_FORMAT_R8_UINT;
            viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(vulkanContext->getDevice(), &viewInfo, nullptr, &voxelImageView[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create texture image view!");
            }
            transitionImageLayout(voxelTexture[i], VK_FORMAT_R8_UINT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL, 1);

            VkBufferCreateInfo bufferCreateInfo = {};
        bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferCreateInfo.size = 128*128*128;
        bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(vulkanContext->getDevice(), &bufferCreateInfo, nullptr, &voxelStagingBuffer[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create voxel staging buffer");
        }

        VkMemoryRequirements stagingMemRequirements;
        vkGetBufferMemoryRequirements(vulkanContext->getDevice(), voxelStagingBuffer[i], &stagingMemRequirements);

        VkMemoryAllocateInfo stagingAllocInfo = {};
        stagingAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        stagingAllocInfo.allocationSize = stagingMemRequirements.size;
        stagingAllocInfo.memoryTypeIndex = findMemoryType(vulkanContext->getPhysicalDevice(), stagingMemRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        vkAllocateMemory(vulkanContext->getDevice(), &stagingAllocInfo, nullptr, &voxelStagingBufferMemory[i]);
        vkBindBufferMemory(vulkanContext->getDevice(), voxelStagingBuffer[i], voxelStagingBufferMemory[i], 0);
        }

        VkSamplerCreateInfo samplerCreateInfo = {};
        samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerCreateInfo.magFilter = VK_FILTER_NEAREST;
        samplerCreateInfo.minFilter = VK_FILTER_NEAREST;
        samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
        samplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;

        vkCreateSampler(vulkanContext->getDevice(), &samplerCreateInfo, nullptr, &voxelTextureSampler);

        startWorkers(10);
        for (int i = 0; i < 512; i++)
        {
            requestChunk(i, 1.0f);
        }
        SortChunks();
        for (int i = 0; i < 513; i++)
        {
            std::lock_guard<std::mutex> lock(queueMutex);
             glm::vec3 cameraGlobal = -cameraPosition + glm::vec3(chunkPosition) * 128.0f;
             uint16_t id = chunkQueue[i];
            printf("CHUNK: x %f y %f z %f x %i y %i z %i %i %f\n", cameraGlobal.x, cameraGlobal.y, cameraGlobal.z, voxelData[id].position.x, voxelData[id].position.y, voxelData[id].position.z, chunkQueue[i], glm::distance(cameraGlobal, glm::vec3(voxelData[id].position)));
        }
        std::this_thread::sleep_for(std::chrono::seconds(5));

{



        // Staging buffer to update the texture
voxelChunkMapStagingBuffer.resize(MAX_FRAMES_IN_FLIGHT);
voxelChunkMapStagingBufferMemory.resize(MAX_FRAMES_IN_FLIGHT);
voxelChunkMapTexture.resize(MAX_FRAMES_IN_FLIGHT);
voxelChunkMapTexturesMemory.resize(MAX_FRAMES_IN_FLIGHT);
voxelChunkMapImageView.resize(MAX_FRAMES_IN_FLIGHT);

for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
{
        // Create the image (texture)
        VkImageCreateInfo voxelChunkMapImageInfo = {};
        voxelChunkMapImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        voxelChunkMapImageInfo.imageType = VK_IMAGE_TYPE_3D;
        voxelChunkMapImageInfo.extent.width = 8;
        voxelChunkMapImageInfo.extent.height = 8;
        voxelChunkMapImageInfo.extent.depth = 8;
        voxelChunkMapImageInfo.mipLevels = 1;
        voxelChunkMapImageInfo.arrayLayers = 1;
        voxelChunkMapImageInfo.format = VK_FORMAT_R16_UINT;
        voxelChunkMapImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        voxelChunkMapImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        voxelChunkMapImageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
        voxelChunkMapImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        voxelChunkMapImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

        if (vkCreateImage(vulkanContext->getDevice(), &voxelChunkMapImageInfo, nullptr, &voxelChunkMapTexture[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture image!");
        }

        // Allocate memory for the image
        VkMemoryRequirements voxelChunkMapMemRequirements;
        vkGetImageMemoryRequirements(vulkanContext->getDevice(), voxelChunkMapTexture[i], &voxelChunkMapMemRequirements);

        VkMemoryAllocateInfo voxelChunkMapAllocInfo = {};
        voxelChunkMapAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        voxelChunkMapAllocInfo.allocationSize = voxelChunkMapMemRequirements.size;
        voxelChunkMapAllocInfo.memoryTypeIndex = findMemoryType(vulkanContext->getPhysicalDevice(), voxelChunkMapMemRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        if (vkAllocateMemory(vulkanContext->getDevice(), &voxelChunkMapAllocInfo, nullptr, &voxelChunkMapTexturesMemory[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate memory for texture image!");
        }

        vkBindImageMemory(vulkanContext->getDevice(), voxelChunkMapTexture[i], voxelChunkMapTexturesMemory[i], 0);

        VkImageViewCreateInfo voxelChunkMapViewInfo = {};
        voxelChunkMapViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        voxelChunkMapViewInfo.image = voxelChunkMapTexture[i];
        voxelChunkMapViewInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
        voxelChunkMapViewInfo.format = VK_FORMAT_R16_UINT;
        voxelChunkMapViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        voxelChunkMapViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        voxelChunkMapViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        voxelChunkMapViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        voxelChunkMapViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        voxelChunkMapViewInfo.subresourceRange.baseMipLevel = 0;
        voxelChunkMapViewInfo.subresourceRange.levelCount = 1;
        voxelChunkMapViewInfo.subresourceRange.baseArrayLayer = 0;
        voxelChunkMapViewInfo.subresourceRange.layerCount = 1;
        voxelChunkMapViewInfo.pNext = nullptr;

        if (vkCreateImageView(vulkanContext->getDevice(), &voxelChunkMapViewInfo, nullptr, &voxelChunkMapImageView[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture image view!");
        }



        VkBufferCreateInfo voxelChunkMapBufferCreateInfo = {};
        voxelChunkMapBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        voxelChunkMapBufferCreateInfo.size = 8*8*8*2;
        voxelChunkMapBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        voxelChunkMapBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(vulkanContext->getDevice(), &voxelChunkMapBufferCreateInfo, nullptr, &voxelChunkMapStagingBuffer[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create voxel staging buffer");
        }

        VkMemoryRequirements voxelChunkMapStagingMemRequirements;
        vkGetBufferMemoryRequirements(vulkanContext->getDevice(), voxelChunkMapStagingBuffer[i], &voxelChunkMapStagingMemRequirements);

        VkMemoryAllocateInfo voxelChunkMapStagingAllocInfo = {};
        voxelChunkMapStagingAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        voxelChunkMapStagingAllocInfo.allocationSize = voxelChunkMapStagingMemRequirements.size;
        voxelChunkMapStagingAllocInfo.memoryTypeIndex = findMemoryType(vulkanContext->getPhysicalDevice(), voxelChunkMapStagingMemRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        vkAllocateMemory(vulkanContext->getDevice(), &voxelChunkMapStagingAllocInfo, nullptr, &voxelChunkMapStagingBufferMemory[i]);
        vkBindBufferMemory(vulkanContext->getDevice(), voxelChunkMapStagingBuffer[i], voxelChunkMapStagingBufferMemory[i], 0);
        
        transitionImageLayout(voxelChunkMapTexture[i], VK_FORMAT_R16_UINT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL, 1);
}
        // Voxel Data

        voxelChunkMapData = new uint16_t[8*8*8]();

        for (int i = 0; i < 8*8*8; i++)
        {
            voxelChunkMapData[i] = i;
        }
}
    }

    void GenerateTerrain()
    {
        // voxelData.resize(512);
        // for (int i = 0; i < 512; i++)
        // {
        //     voxelData[i] = {};
        //     voxelData[i].data = new uint8_t[128 * 128 * 128]();
            
        // }

        // for (int x = 0; x < 8; x++)
        // {
        //     for (int y = 0; y < 8; y++)
        //     {

        //     }
        // }
        // Voxel Data
        auto it = std::chrono::system_clock::now();
#define vox(x,y,z) voxelData[(int)std::floor((float)(z) / 128.0f) * 64 + (int)std::floor((float)(y) / 128.0f) * 8 + (int)std::floor((float)(x) / 128.0f)].data[((z) % 128) * 128 * 128 + ((y) % 128) * 128 + ((x) % 128)]
#define voxChunk(chunk, x,y,z) chunk[((z) % 128) * 128 * 128 + ((y) % 128) * 128 + ((x) % 128)]


#define TERRAIN_SCALE 2
#define TERRAIN_AREA (1024)

        //printf("hwc: %i", std::thread::hardware_concurrency());
        printf("Generating\n");
        const int WORKERS = 20;
        const int SLICE_THICKNESS = TERRAIN_AREA / WORKERS;

        std::thread* threads[WORKERS]{};

        for (int i = 0; i < WORKERS; i++)
        {
            // threads[i] = new std::thread{&TerrainWork, voxelData, &noise, i * SLICE_THICKNESS, SLICE_THICKNESS};
        }

        puts("");
        for (int i = 0; i < WORKERS; i++)
        {
            threads[i]->join();
            printf("\rProgress: %.1f", (float)i / WORKERS * 100);
        }

        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - it);
        printf("\nTIME: %lli\n\n", ms.count());

                bool idk = false;

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

    std::vector<uint16_t> chunkUpdateQueue;

    void UpdateVoxels(VkCommandBuffer commandBuffer, uint8_t currentImage)
    {
        for (int i = 0; i < 16; i++)
        {
            if (chunkUpdateQueue[0] == 0)
            {
                break;
            }
            
            uint16_t ID = chunkUpdateQueue[chunkUpdateQueue[0]--];
            transitionImageLayout(commandBuffer, voxelTexture[ID], VK_FORMAT_R8_UINT, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1);

            void* mappedData;
            vkMapMemory(vulkanContext->getDevice(), voxelStagingBufferMemory[ID], 0, 128*128*128, 0, &mappedData);
            memcpy(mappedData, (char*)voxelData[ID].data, 128*128*128);
            vkUnmapMemory(vulkanContext->getDevice(), voxelStagingBufferMemory[ID]);
            VkBufferImageCopy region = {};
            region.bufferOffset = 0;
            region.bufferRowLength = 0;
            region.bufferImageHeight = 0;
            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.imageSubresource.mipLevel = 0;
            region.imageSubresource.baseArrayLayer = 0;
            region.imageSubresource.layerCount = 1;
            region.imageOffset = { 0, 0, 0 };
            region.imageExtent = { 128, 128, 128 };
            vkCmdCopyBufferToImage(commandBuffer, voxelStagingBuffer[ID], voxelTexture[ID], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
            transitionImageLayout(commandBuffer, voxelTexture[ID], VK_FORMAT_R8_UINT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL, 1);
        }



        


        void* mappedVoxelChunkMapData;
        vkMapMemory(vulkanContext->getDevice(), voxelChunkMapStagingBufferMemory[currentImage], 0, 8*8*8*2, 0, &mappedVoxelChunkMapData);
        memcpy(mappedVoxelChunkMapData, voxelChunkMapData, 8*8*8*2);
        vkUnmapMemory(vulkanContext->getDevice(), voxelChunkMapStagingBufferMemory[currentImage]);

        VkBufferImageCopy voxelChunkMapRegion = {};
        voxelChunkMapRegion.bufferOffset = 0;
        voxelChunkMapRegion.bufferRowLength = 0;
        voxelChunkMapRegion.bufferImageHeight = 0;
        voxelChunkMapRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        voxelChunkMapRegion.imageSubresource.mipLevel = 0;
        voxelChunkMapRegion.imageSubresource.baseArrayLayer = 0;
        voxelChunkMapRegion.imageSubresource.layerCount = 1;
        voxelChunkMapRegion.imageOffset = { 0,0,0 };
        voxelChunkMapRegion.imageExtent = { 8,8,8 };

        vkCmdCopyBufferToImage(commandBuffer, voxelChunkMapStagingBuffer[currentImage], voxelChunkMapTexture[currentImage], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &voxelChunkMapRegion);
    }

    void createRaytracingPipeline()
    {
        auto raygenShaderCode = readFile("rgen.spv");
        auto missShaderCode = readFile("rmiss.spv");

        VkShaderModule raygenShaderModule = createShaderModule(raygenShaderCode);
        VkShaderModule missShaderModule = createShaderModule(missShaderCode);

        VkPipelineShaderStageCreateInfo raygenShaderStageInfo{};
        raygenShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        raygenShaderStageInfo.stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        raygenShaderStageInfo.module = raygenShaderModule;
        raygenShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo missShaderStageInfo{};
        missShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        missShaderStageInfo.stage = VK_SHADER_STAGE_MISS_BIT_KHR;
        missShaderStageInfo.module = missShaderModule;
        missShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = {raygenShaderStageInfo, missShaderStageInfo};

        VkRayTracingShaderGroupCreateInfoKHR shaderGroups[2] = {};

        shaderGroups[0].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        shaderGroups[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        shaderGroups[0].generalShader = 0;
        shaderGroups[0].closestHitShader = VK_SHADER_UNUSED_KHR;
        shaderGroups[0].anyHitShader = VK_SHADER_UNUSED_KHR;
        shaderGroups[0].intersectionShader = VK_SHADER_UNUSED_KHR;

        shaderGroups[1].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        shaderGroups[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        shaderGroups[1].generalShader = 1;
        shaderGroups[1].closestHitShader = VK_SHADER_UNUSED_KHR;
        shaderGroups[1].anyHitShader = VK_SHADER_UNUSED_KHR;
        shaderGroups[1].intersectionShader = VK_SHADER_UNUSED_KHR;


        VkDescriptorSetLayoutBinding storageImageBinding = {};
        storageImageBinding.binding = 0;
        storageImageBinding.descriptorCount = 1;
        storageImageBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        storageImageBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        storageImageBinding.pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutBinding uniformBinding = {};
        uniformBinding.binding = 1;
        uniformBinding.descriptorCount = 1;
        uniformBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uniformBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        uniformBinding.pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutBinding positionStorageImageBinding = {};
        positionStorageImageBinding.binding = 2;
        positionStorageImageBinding.descriptorCount = 1;
        positionStorageImageBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        positionStorageImageBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        positionStorageImageBinding.pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutBinding voxelTexture = {};
        voxelTexture.binding = 3;
        voxelTexture.descriptorCount = 512;
        voxelTexture.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        voxelTexture.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        voxelTexture.pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutBinding voxelChunkMapTexture = {};
        voxelChunkMapTexture.binding = 4;
        voxelChunkMapTexture.descriptorCount = 1;
        voxelChunkMapTexture.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        voxelChunkMapTexture.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        voxelChunkMapTexture.pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutBinding lightStorageImageBindingX = {};
        lightStorageImageBindingX.binding = 5;
        lightStorageImageBindingX.descriptorCount = 1;
        lightStorageImageBindingX.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        lightStorageImageBindingX.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        lightStorageImageBindingX.pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutBinding lightStorageImageBindingY = {};
        lightStorageImageBindingY.binding = 6;
        lightStorageImageBindingY.descriptorCount = 1;
        lightStorageImageBindingY.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        lightStorageImageBindingY.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        lightStorageImageBindingY.pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutBinding lightStorageImageBindingZ = {};
        lightStorageImageBindingZ.binding = 7;
        lightStorageImageBindingZ.descriptorCount = 1;
        lightStorageImageBindingZ.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        lightStorageImageBindingZ.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        lightStorageImageBindingZ.pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutBinding lightStorageImageBindingW = {};
        lightStorageImageBindingW.binding = 8;
        lightStorageImageBindingW.descriptorCount = 1;
        lightStorageImageBindingW.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        lightStorageImageBindingW.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        lightStorageImageBindingW.pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutBinding bindings[] = {storageImageBinding, uniformBinding, voxelTexture, positionStorageImageBinding, voxelChunkMapTexture, lightStorageImageBindingX, lightStorageImageBindingY, lightStorageImageBindingZ, lightStorageImageBindingW};

        VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {};
        layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutCreateInfo.bindingCount = 9;
        layoutCreateInfo.pBindings = bindings;
        layoutCreateInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;

        VkDescriptorBindingFlags bindless_flags = 
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT |
                VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT;
                VkDescriptorBindingFlags flags[] = {0, 0, bindless_flags, 0, 0, 0, 0, 0, 0};

        VkDescriptorSetLayoutBindingFlagsCreateInfoEXT extended_info{
            VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT,
            nullptr};
        extended_info.bindingCount = 9;
        extended_info.pBindingFlags = flags;

        layoutCreateInfo.pNext = &extended_info;

        if (vkCreateDescriptorSetLayout(vulkanContext->getDevice(), &layoutCreateInfo, nullptr, &raytracingDescriptorSetLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create raytracing descriptor set layout!");
        }

                VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        pushConstantRange.offset = 0;
        pushConstantRange.size = 8;

        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
        pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCreateInfo.setLayoutCount = 1;
        pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;
        pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
        pipelineLayoutCreateInfo.pSetLayouts = &raytracingDescriptorSetLayout;

        if (vkCreatePipelineLayout(vulkanContext->getDevice(), &pipelineLayoutCreateInfo, nullptr, &raytracingPipelineLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create raytracing pipeline layout!");
        }

        printf("PIPELINE LAYOUT %i\n", 1);

        VkRayTracingPipelineCreateInfoKHR pipelineCreateInfo{};
        pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
        pipelineCreateInfo.stageCount = 2;
        pipelineCreateInfo.pStages = shaderStages;
        pipelineCreateInfo.groupCount = 2;
        pipelineCreateInfo.pGroups = shaderGroups;
        pipelineCreateInfo.maxPipelineRayRecursionDepth = 1;
        pipelineCreateInfo.layout = raytracingPipelineLayout;

        PFN_vkCreateRayTracingPipelinesKHR vkCreateRayTracingPipelinesKHR = reinterpret_cast<PFN_vkCreateRayTracingPipelinesKHR>(vkGetDeviceProcAddr(vulkanContext->getDevice(), "vkCreateRayTracingPipelinesKHR"));
        if (vkCreateRayTracingPipelinesKHR(vulkanContext->getDevice(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &raytracingPipeline) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create raytracing pipeline!");
        }
        printf("PIPELINE LAYOUT %i\n", 1);
        vkDestroyShaderModule(vulkanContext->getDevice(), raygenShaderModule, nullptr);
        vkDestroyShaderModule(vulkanContext->getDevice(), missShaderModule, nullptr);


        // Pool

        VkDescriptorPoolSize storageImagePoolSize = {};
        storageImagePoolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        storageImagePoolSize.descriptorCount = 2;

        VkDescriptorPoolSize uboPoolSize = {};
        uboPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboPoolSize.descriptorCount = 2;

        VkDescriptorPoolSize upperVoxelPoolSize = {};
        upperVoxelPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        upperVoxelPoolSize.descriptorCount = 600;

        VkDescriptorPoolSize poolSizes[] = {storageImagePoolSize, uboPoolSize, upperVoxelPoolSize};

        VkDescriptorPoolCreateInfo poolCreateInfo = {};
        poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolCreateInfo.poolSizeCount = 3;
        poolCreateInfo.pPoolSizes = poolSizes;
        poolCreateInfo.maxSets = 50;

        if (vkCreateDescriptorPool(vulkanContext->getDevice(), &poolCreateInfo, nullptr, &raytracingDescriptorPool) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create raytracing descriptor pool!");
        }

        // Updating Descriptor Set

        raytracingDescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);

        VkDescriptorSetLayout setLayouts[] = {raytracingDescriptorSetLayout, raytracingDescriptorSetLayout};

        VkDescriptorSetAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = raytracingDescriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        allocInfo.pSetLayouts = setLayouts;
        
        if (vkAllocateDescriptorSets(vulkanContext->getDevice(), &allocInfo, raytracingDescriptorSets.data()))
        {
            throw std::runtime_error("Failed to create raytracing descriptor set!");
        }

        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            VkDescriptorImageInfo positionImageInfo = {};
            positionImageInfo.imageView = raytracingPositionStorageImageView[i];
            positionImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

            VkWriteDescriptorSet writePositionStorageDescriptorSet = {};
            writePositionStorageDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writePositionStorageDescriptorSet.dstSet = raytracingDescriptorSets[i];
            writePositionStorageDescriptorSet.dstBinding = 2;
            writePositionStorageDescriptorSet.dstArrayElement = 0;
            writePositionStorageDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            writePositionStorageDescriptorSet.descriptorCount = 1;
            writePositionStorageDescriptorSet.pImageInfo = &positionImageInfo;

            VkDescriptorImageInfo lightImageInfoX = {};
            lightImageInfoX.imageView = raytracingLightStorageImageViewX[i];
            lightImageInfoX.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

            VkDescriptorImageInfo lightImageInfoY = {};
            lightImageInfoY.imageView = raytracingLightStorageImageViewY[i];
            lightImageInfoY.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

            VkDescriptorImageInfo lightImageInfoZ = {};
            lightImageInfoZ.imageView = raytracingLightStorageImageViewZ[i];
            lightImageInfoZ.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

            VkDescriptorImageInfo lightImageInfoW = {};
            lightImageInfoW.imageView = raytracingLightStorageImageViewW[i];
            lightImageInfoW.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

            VkWriteDescriptorSet writeLightStorageDescriptorSetX = {};
            writeLightStorageDescriptorSetX.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeLightStorageDescriptorSetX.dstSet = raytracingDescriptorSets[i];
            writeLightStorageDescriptorSetX.dstBinding = 5;
            writeLightStorageDescriptorSetX.dstArrayElement = 0;
            writeLightStorageDescriptorSetX.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            writeLightStorageDescriptorSetX.descriptorCount = 1;
            writeLightStorageDescriptorSetX.pImageInfo = &lightImageInfoX;

            VkWriteDescriptorSet writeLightStorageDescriptorSetY = {};
            writeLightStorageDescriptorSetY.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeLightStorageDescriptorSetY.dstSet = raytracingDescriptorSets[i];
            writeLightStorageDescriptorSetY.dstBinding = 6;
            writeLightStorageDescriptorSetY.dstArrayElement = 0;
            writeLightStorageDescriptorSetY.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            writeLightStorageDescriptorSetY.descriptorCount = 1;
            writeLightStorageDescriptorSetY.pImageInfo = &lightImageInfoY;

            VkWriteDescriptorSet writeLightStorageDescriptorSetZ = {};
            writeLightStorageDescriptorSetZ.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeLightStorageDescriptorSetZ.dstSet = raytracingDescriptorSets[i];
            writeLightStorageDescriptorSetZ.dstBinding = 7;
            writeLightStorageDescriptorSetZ.dstArrayElement = 0;
            writeLightStorageDescriptorSetZ.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            writeLightStorageDescriptorSetZ.descriptorCount = 1;
            writeLightStorageDescriptorSetZ.pImageInfo = &lightImageInfoZ;

            VkWriteDescriptorSet writeLightStorageDescriptorSetW = {};
            writeLightStorageDescriptorSetW.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeLightStorageDescriptorSetW.dstSet = raytracingDescriptorSets[i];
            writeLightStorageDescriptorSetW.dstBinding = 8;
            writeLightStorageDescriptorSetW.dstArrayElement = 0;
            writeLightStorageDescriptorSetW.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            writeLightStorageDescriptorSetW.descriptorCount = 1;
            writeLightStorageDescriptorSetW.pImageInfo = &lightImageInfoW;

            VkDescriptorImageInfo imageInfo = {};
            imageInfo.imageView = raytracingStorageImageView[i];
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

            VkWriteDescriptorSet writeStorageDescriptorSet = {};
            writeStorageDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeStorageDescriptorSet.dstSet = raytracingDescriptorSets[i];
            writeStorageDescriptorSet.dstBinding = 0;
            writeStorageDescriptorSet.dstArrayElement = 0;
            writeStorageDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            writeStorageDescriptorSet.descriptorCount = 1;
            writeStorageDescriptorSet.pImageInfo = &imageInfo;

            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = uniformBuffer[i];
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(TransformUBO);

            VkWriteDescriptorSet writeTransformDescriptorSet = {};
            writeTransformDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeTransformDescriptorSet.dstSet = raytracingDescriptorSets[i];
            writeTransformDescriptorSet.dstBinding = 1;
            writeTransformDescriptorSet.dstArrayElement = 0;
            writeTransformDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            writeTransformDescriptorSet.descriptorCount = 1;
            writeTransformDescriptorSet.pBufferInfo = &bufferInfo;

            std::vector<VkDescriptorImageInfo> voxelImageInfos(512);

            for (int i = 0; i < 512; i++)
            {
                voxelImageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                voxelImageInfos[i].imageView = voxelImageView[i];
                voxelImageInfos[i].sampler = voxelTextureSampler;
            }
           

            VkWriteDescriptorSet voxelDescriptorSet = {};
            voxelDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            voxelDescriptorSet.dstSet = raytracingDescriptorSets[i];
            voxelDescriptorSet.dstBinding = 3;
            voxelDescriptorSet.dstArrayElement = 0;
            voxelDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            voxelDescriptorSet.descriptorCount = 512;
            voxelDescriptorSet.pImageInfo = voxelImageInfos.data();


            VkDescriptorImageInfo voxelChunkMapImageInfo = {};
            voxelChunkMapImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            voxelChunkMapImageInfo.imageView = voxelChunkMapImageView[i];
            voxelChunkMapImageInfo.sampler = voxelTextureSampler;

            VkWriteDescriptorSet voxelChunkMapDescriptorSet = {};
            voxelChunkMapDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            voxelChunkMapDescriptorSet.dstSet = raytracingDescriptorSets[i];
            voxelChunkMapDescriptorSet.dstBinding = 4;
            voxelChunkMapDescriptorSet.dstArrayElement = 0;
            voxelChunkMapDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            voxelChunkMapDescriptorSet.descriptorCount = 1;
            voxelChunkMapDescriptorSet.pImageInfo = &voxelChunkMapImageInfo;

            VkWriteDescriptorSet writeDescriptorSets[] = {writeStorageDescriptorSet, writeTransformDescriptorSet, voxelDescriptorSet, writeLightStorageDescriptorSetX, writeLightStorageDescriptorSetY, writeLightStorageDescriptorSetZ, writeLightStorageDescriptorSetW, writePositionStorageDescriptorSet, voxelChunkMapDescriptorSet};

            vkUpdateDescriptorSets(vulkanContext->getDevice(), 9, writeDescriptorSets, 0, nullptr);
        }

    }

    void createGraphicsPipeline() {
        auto vertShaderCode = readFile("vert.spv");
        auto fragShaderCode = readFile("frag.spv");

        VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
        VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 0;
        vertexInputInfo.vertexAttributeDescriptionCount = 0;

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        // pipline binding

        VkDescriptorSetLayoutBinding binding = {};
        binding.binding = 0;
        binding.descriptorCount = 1;
        binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        binding.pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {};
        layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutCreateInfo.bindingCount = 1;
        layoutCreateInfo.pBindings = &binding;
        layoutCreateInfo.pNext = nullptr;

        if (vkCreateDescriptorSetLayout(vulkanContext->getDevice(), &layoutCreateInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create raytracing descriptor set layout!");
        }



        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

        if (vkCreatePipelineLayout(vulkanContext->getDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = pipelineLayout;
        pipelineInfo.renderPass = vulkanContext->getRenderPass();
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        if (vkCreateGraphicsPipelines(vulkanContext->getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics pipeline!");
        }

        vkDestroyShaderModule(vulkanContext->getDevice(), fragShaderModule, nullptr);
        vkDestroyShaderModule(vulkanContext->getDevice(), vertShaderModule, nullptr);

         VkDescriptorPoolSize poolSize = {};
        poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSize.descriptorCount = 2;

        VkDescriptorPoolCreateInfo poolCreateInfo = {};
        poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolCreateInfo.poolSizeCount = 1;
        poolCreateInfo.pPoolSizes = &poolSize;
        poolCreateInfo.maxSets = 50;

        if (vkCreateDescriptorPool(vulkanContext->getDevice(), &poolCreateInfo, nullptr, &descriptorPool) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create raytracing descriptor pool!");
        }

        




        descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);

        VkDescriptorSetLayout setLayouts[] = {descriptorSetLayout, descriptorSetLayout};

        VkDescriptorSetAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        allocInfo.pSetLayouts = setLayouts;
        
        if (vkAllocateDescriptorSets(vulkanContext->getDevice(), &allocInfo, descriptorSets.data()))
        {
            throw std::runtime_error("Failed to create raytracing descriptor set!");
        }

        imageSampler.resize(MAX_FRAMES_IN_FLIGHT);

        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {



            VkSamplerCreateInfo samplerCreateInfo = {};
            samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
            samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
            samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_TRANSPARENT_BLACK;
            samplerCreateInfo.anisotropyEnable = VK_FALSE;
            samplerCreateInfo.maxAnisotropy = 1.0f;
            samplerCreateInfo.compareEnable = VK_FALSE;
            samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
            samplerCreateInfo.minLod = 0.0f;
            samplerCreateInfo.maxLod = VK_LOD_CLAMP_NONE;
            samplerCreateInfo.mipLodBias = 0.0f;

            if (vkCreateSampler(vulkanContext->getDevice(), &samplerCreateInfo, nullptr, &imageSampler[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create sampler");
            }
            

            VkDescriptorImageInfo imageInfo = {};
            imageInfo.imageView = raytracingStorageImageView[i];
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
            imageInfo.sampler = imageSampler[i];

            VkWriteDescriptorSet writeDescriptorSet = {};
            writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeDescriptorSet.dstSet = descriptorSets[i];
            writeDescriptorSet.dstBinding = 0;
            writeDescriptorSet.dstArrayElement = 0;
            writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            writeDescriptorSet.descriptorCount = 1;
            writeDescriptorSet.pImageInfo = &imageInfo;

            vkUpdateDescriptorSets(vulkanContext->getDevice(), 1, &writeDescriptorSet, 0, nullptr);
        }

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
                

      
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, raytracingPipeline);

        vkCmdBindDescriptorSets(
            commandBuffer,
            VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
            raytracingPipelineLayout,
            0,
            1,
            &raytracingDescriptorSets[currentFrame],
            0,
            nullptr
        );

        PFN_vkCmdTraceRaysKHR vkCmdTraceRaysKHR = reinterpret_cast<PFN_vkCmdTraceRaysKHR>(vkGetDeviceProcAddr(vulkanContext->getDevice(), "vkCmdTraceRaysKHR"));

        PushConstant c;
        c.flag = 0;
        c.frame = frame[imageIndex];
        vkCmdPushConstants(
            commandBuffer,
            raytracingPipelineLayout,
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
            raytracingPipelineLayout,
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
            raytracingPipelineLayout,
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


        transitionImageLayout(commandBuffer, voxelChunkMapTexture[imageIndex], VK_FORMAT_R16_UINT, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1);
        UpdateVoxels(commandBuffer, imageIndex);
        transitionImageLayout(commandBuffer, voxelChunkMapTexture[imageIndex], VK_FORMAT_R16_UINT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL, 1);
        
        

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

            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

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
                pipelineLayout,
                0,
                1,
                &descriptorSets[currentFrame],
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

    VkCommandBuffer beginSingleTimeCommands() {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = commandManager->getCommandPool();
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(vulkanContext->getDevice(), &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        return commandBuffer;
    }

    void endSingleTimeCommands(VkCommandBuffer commandBuffer) {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(vulkanContext->getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(vulkanContext->getGraphicsQueue());

        vkFreeCommandBuffers(vulkanContext->getDevice(), commandManager->getCommandPool(), 1, &commandBuffer);
    }

    uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        throw std::runtime_error("failed to find suitable memory type!");
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
            allocInfo.memoryTypeIndex = findMemoryType(vulkanContext->getPhysicalDevice(), memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

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

    void updateVoxelChunkMap(int modValue, int offset) {

    switch (offset)
    {
    case 1:
        chunkPosition += glm::ivec3(1, 0, 0);
        break;
    case 7:
        chunkPosition -= glm::ivec3(1, 0, 0);
        break;
    case 8:
        chunkPosition += glm::ivec3(0, 1, 0);
        break;
    case 56:
        chunkPosition += glm::ivec3(0, -1, 0);
        break;  
    case 64:
        chunkPosition += glm::ivec3(0, 0, 1);
        break;
    case 448:
        chunkPosition += glm::ivec3(0, 0, -1);
        break;  
    
    default:
        break;
    }

    const int chunkSize = 8 * 128; // 1024
    for (int z = 0; z < 8; z++) {
        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                int index = z * 8 * 8 + y * 8 + x;
                int chunk = voxelChunkMapData[index];
                int chunkAxis = chunk % modValue;

                uint16_t chunkID = (chunk - chunkAxis) + (chunkAxis + offset) % modValue;
                switch (modValue) {
                    case 8:
                        if (offset == 1 && x == 7)
                        {
                            voxelData[chunkID].position.x += chunkSize;
                            requestChunk(chunkID, 0.0f);
                        }
                        else if (offset == 7 && x == 0)
                        {
                            voxelData[chunkID].position.x -= chunkSize;
                            requestChunk(chunkID, 0.0f);
                        }
                        break;
                    case 64:
                        if (offset == 8 && y == 7)
                        {
                            voxelData[chunkID].position.y += chunkSize;
                            requestChunk(chunkID, 0.0f);
                        }
                        else if (offset == 56 && y == 0)
                        {
                            voxelData[chunkID].position.y -= chunkSize;
                            requestChunk(chunkID, 0.0f);
                        }
                        break;
                    case 512:
                        if (offset == 64 && z == 7)
                        {
                            voxelData[chunkID].position.z += chunkSize;
                            requestChunk(chunkID, 0.0f);
                        }
                        else if (offset == 448 && z == 0)
                        {
                            voxelData[chunkID].position.z -= chunkSize;
                            requestChunk(chunkID, 0.0f);
                        }
                        break;
                }

                voxelChunkMapData[index] = chunkID;
            }
        }
    }
}

    bool firstMouse = true;

    // std::priority_queue<std::pair<float, uint16_t>, 
    //                 std::vector<std::pair<float, uint16_t>>, 
    //                 std::greater<>> chunkQueue;

    std::vector<uint16_t> chunkQueue; // first index is the offset (stack)

    std::unordered_set<uint16_t> activeChunks;
    std::mutex queueMutex;
    std::condition_variable queueCond;
    bool stopThreads = false;

    float randomFromPosition(glm::ivec3 position)
    {
        uint32_t combined = (position.z * 128 * 128 + position.y * 128 + position.x);

        std::mt19937 rng(combined);
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        return dist(rng);
    }
    
    void generateChunk(VoxelChunk& chunk)
    {
        glm::ivec3 position = chunk.position;
        uint8_t* chunkData = chunk.data;
        #pragma omp parallel for collapse(2)
        for (int z = 0; z < 128; z++)
        {
            for (int y = 0; y < 128; y++)
            {
                for (int x = 0; x < 128; x++)
                {
                    glm::ivec3 samplePosition = position + glm::ivec3(x, y, z);
                    uint8_t base = 0;//0b11100000;
                    float v = (noise.GetNoise(samplePosition.x * 0.1f * TERRAIN_SCALE, samplePosition.y * 0.1f * TERRAIN_SCALE, samplePosition.z * 0.4f * TERRAIN_SCALE) + noise.GetNoise(samplePosition.x * 1.5f * TERRAIN_SCALE, samplePosition.y * 1.5f * TERRAIN_SCALE, samplePosition.z * 1.0f * TERRAIN_SCALE) * 0.03) / 1.03f;
                    float v_above = (noise.GetNoise(samplePosition.x * 0.1f * TERRAIN_SCALE, samplePosition.y * 0.1f * TERRAIN_SCALE, (samplePosition.z-1) * 0.4f * TERRAIN_SCALE) + noise.GetNoise(samplePosition.x * 1.5f * TERRAIN_SCALE, samplePosition.y * 1.5f * TERRAIN_SCALE, (samplePosition.z-1) * 1.0f * TERRAIN_SCALE) * 0.03) / 1.03f;
                    int material = v <= 0.1;
                    if (material != MAT_AIR)
                    {
                        if ((v_above <= 0.1))
                        {
                            float v = noise.GetNoise(x * 10.0f * TERRAIN_SCALE, y * 10.0f * TERRAIN_SCALE, z * 10.0f * TERRAIN_SCALE) * 0.7f + noise.GetNoise(x * 5.0f * TERRAIN_SCALE, y * 5.0f * TERRAIN_SCALE, z * 5.0f * TERRAIN_SCALE) * 0.3f;
                            if (v > 0.33)
                            {
                                material = MAT_STONE;
                            }
                            else if (v > -0.33)
                            {
                                material = MAT_STONE2;
                            }
                            else
                            {
                                material = MAT_STONE3;
                            }
                        }
                      
                    }

                    voxChunk(chunkData, x, y, z) = base | material;
                }
            }
        }

        // Set the base
        #pragma omp parallel for collapse(2)
        for (int x = 0; x < 128; x+=2)
        {
            for (int y = 0; y < 128; y+=2)
            {
                for (int z = 0; z < 128; z+=2)
                {
                    if (voxel_mat(voxChunk(chunkData, x, y, z)) == MAT_AIR && 
                        voxel_mat(voxChunk(chunkData, x+1, y, z)) == MAT_AIR && 
                        voxel_mat(voxChunk(chunkData, x, y+1, z)) == MAT_AIR && 
                        voxel_mat(voxChunk(chunkData, x, y, z+1)) == MAT_AIR && 
                        voxel_mat(voxChunk(chunkData, x+1, y, z+1)) == MAT_AIR && 
                        voxel_mat(voxChunk(chunkData, x+1, y+1, z)) == MAT_AIR && 
                        voxel_mat(voxChunk(chunkData, x, y+1, z+1)) == MAT_AIR && 
                        voxel_mat(voxChunk(chunkData, x+1, y+1, z+1)) == MAT_AIR)
                    {
                        uint8_t value = 1 << 5;
                        voxChunk(chunkData, x, y, z) = value;
                        voxChunk(chunkData, x+1, y, z) = value;
                        voxChunk(chunkData, x, y+1, z) = value;
                        voxChunk(chunkData, x, y, z+1) = value;
                        voxChunk(chunkData, x+1, y, z+1) = value;
                        voxChunk(chunkData, x+1, y+1, z) = value;
                        voxChunk(chunkData, x, y+1, z+1) = value;
                        voxChunk(chunkData, x+1, y+1, z+1) = value;
                    }
                }
            }
        }
        #pragma omp parallel for collapse(2)
        for (int x = 0; x < 128; x+=4)
        {
            for (int y = 0; y < 128; y+=4)
            {
                for (int z = 0; z < 128; z+=4)
                {
                    if (voxel_base(voxChunk(chunkData, x, y, z)) == 1 && 
                        voxel_base(voxChunk(chunkData, x+2, y, z)) == 1 && 
                        voxel_base(voxChunk(chunkData, x, y+2, z)) == 1 && 
                        voxel_base(voxChunk(chunkData, x, y, z+2)) == 1 && 
                        voxel_base(voxChunk(chunkData, x+2, y, z+2)) == 1 && 
                        voxel_base(voxChunk(chunkData, x+2, y+2, z)) == 1 && 
                        voxel_base(voxChunk(chunkData, x, y+2, z+2)) == 1 && 
                        voxel_base(voxChunk(chunkData, x+2, y+2, z+2)) == 1)
                    {
                        uint8_t value = 2 << 5;
                        for (int x2 = 0; x2 < 4; x2++)
                        {
                            for (int y2 = 0; y2 < 4; y2++)
                            {
                                for (int z2 = 0; z2 < 4; z2++)
                                {
                                    voxChunk(chunkData, x+x2, y+y2, z+z2) = value;
                                }
                            }
                        }
                    }
                }
            }
        }

        #pragma omp parallel for collapse(2)
        for (int x = 0; x < 128; x+=8)
        {
            for (int y = 0; y < 128; y+=8)
            {
                for (int z = 0; z < 128; z+=8)
                {
                    if (voxel_base(voxChunk(chunkData, x, y, z)) == 2 && 
                        voxel_base(voxChunk(chunkData, x+4, y, z)) == 2 && 
                        voxel_base(voxChunk(chunkData, x, y+4, z)) == 2 && 
                        voxel_base(voxChunk(chunkData, x, y, z+4)) == 2 && 
                        voxel_base(voxChunk(chunkData, x+4, y, z+4)) == 2 && 
                        voxel_base(voxChunk(chunkData, x+4, y+4, z)) == 2 && 
                        voxel_base(voxChunk(chunkData, x, y+4, z+4)) == 2 && 
                        voxel_base(voxChunk(chunkData, x+4, y+4, z+4)) == 2)
                    {
                        uint8_t value = 3 << 5;
                        for (int x2 = 0; x2 < 8; x2++)
                        {
                            for (int y2 = 0; y2 < 8; y2++)
                            {
                                for (int z2 = 0; z2 < 8; z2++)
                                {
                                    voxChunk(chunkData, x+x2, y+y2, z+z2) = value;
                                }
                            }
                        }
                    }
                }
            }
        }
        #pragma omp parallel for collapse(2)
        for (int x = 0; x < 128; x+=16)
        {
            for (int y = 0; y < 128; y+=16)
            {
                for (int z = 0; z < 128; z+=16)
                {
                    if (voxel_base(voxChunk(chunkData, x, y, z)) == 3 && 
                        voxel_base(voxChunk(chunkData, x+8, y, z)) == 3 && 
                        voxel_base(voxChunk(chunkData, x, y+8, z)) == 3 && 
                        voxel_base(voxChunk(chunkData, x, y, z+8)) == 3 && 
                        voxel_base(voxChunk(chunkData, x+8, y, z+8)) == 3 && 
                        voxel_base(voxChunk(chunkData, x+8, y+8, z)) == 3 && 
                        voxel_base(voxChunk(chunkData, x, y+8, z+8)) == 3 && 
                        voxel_base(voxChunk(chunkData, x+8, y+8, z+8)) == 3)
                    {
                        uint8_t value = 4 << 5;
                        for (int x2 = 0; x2 < 16; x2++)
                        {
                            for (int y2 = 0; y2 < 16; y2++)
                            {
                                for (int z2 = 0; z2 < 16; z2++)
                                {
                                    voxChunk(chunkData, x+x2, y+y2, z+z2) = value;
                                }
                            }
                        }
                    }
                }
            }
        }
        #pragma omp parallel for collapse(2)
        for (int x = 0; x < 128; x+=32)
        {
            for (int y = 0; y < 128; y+=32)
            {
                for (int z = 0; z < 128; z+=32)
                {
                    if (voxel_base(voxChunk(chunkData, x, y, z)) == 4 && 
                        voxel_base(voxChunk(chunkData, x+16, y, z)) == 4 && 
                        voxel_base(voxChunk(chunkData, x, y+16, z)) == 4 && 
                        voxel_base(voxChunk(chunkData, x, y, z+16)) == 4 && 
                        voxel_base(voxChunk(chunkData, x+16, y, z+16)) == 4 && 
                        voxel_base(voxChunk(chunkData, x+16, y+16, z)) == 4 && 
                        voxel_base(voxChunk(chunkData, x, y+16, z+16)) == 4 && 
                        voxel_base(voxChunk(chunkData, x+16, y+16, z+16)) == 4)
                    {
                        uint8_t value = 5 << 5;
                        for (int x2 = 0; x2 < 32; x2++)
                        {
                            for (int y2 = 0; y2 < 32; y2++)
                            {
                                for (int z2 = 0; z2 < 32; z2++)
                                {
                                    voxChunk(chunkData, x+x2, y+y2, z+z2) = value;
                                }
                            }
                        }
                    }
                }
            }
        }


    }

    void chunkWorker() {
        while (!stopThreads) {
            uint16_t chunkID;
            uint32_t genID;

            {
                std::unique_lock<std::mutex> lock(queueMutex);
                queueCond.wait(lock, [&] { return chunkQueue[0] != 0 || stopThreads; });

                if (stopThreads) return;

                if (chunkQueue[0] > 0) {
                    chunkID = chunkQueue[chunkQueue[0]--];
                } else {
                    continue;  // Recheck condition
                }
            }
            voxelData[chunkID].inQueue = false;

            generateChunk(voxelData[chunkID]);
            chunkUpdateQueue[++chunkUpdateQueue[0]] = chunkID;
        }
    }

    void requestChunk(uint16_t chunkID, float priority) {
        // std::fill(voxelData[chunkID].data, voxelData[chunkID].data + 128 * 128 * 128, 0);
        std::lock_guard<std::mutex> lock(queueMutex);

        // chunkGenerationID[chunkID]++; // Increment generation ID
        if (voxelData[chunkID].inQueue) return;
        chunkQueue[++chunkQueue[0]] = chunkID;
        voxelData[chunkID].inQueue = true;
        queueCond.notify_one();
    }

    bool ChunkSort(uint16_t a, uint16_t b)
    {
        glm::vec3 cameraGlobal = (glm::vec3(chunkPosition) * 128.0f) - cameraPosition;
        return glm::distance(cameraGlobal, glm::vec3(voxelData[a].position)) > glm::distance(cameraGlobal, glm::vec3(voxelData[b].position));
    }

    void SortChunks()
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        if (chunkQueue[0] > 5)
        std::sort(chunkQueue.begin() + 1, chunkQueue.begin() + chunkQueue[0], [this](uint16_t a, uint16_t b){return ChunkSort(a,b);});
    }

    std::vector<std::thread> workers;
    void startWorkers(int numThreads) {
        voxelData.resize(512);
        chunkQueue.resize(513);
        chunkQueue[0] = 0;
        chunkUpdateQueue.resize(513);
        chunkUpdateQueue[0] = 0;

        for (int x = 0; x < 8; x++)
        {
            for (int y = 0; y < 8; y++)
            {
                for (int z = 0; z < 8; z++)
                {
                    size_t chunkIndex = z*64 + y*8 + x;
                    voxelData[chunkIndex] = {};
                    voxelData[chunkIndex].data = new uint8_t[128 * 128 * 128]();
                    voxelData[chunkIndex].position = glm::ivec3(x * 128, y * 128, z * 128);
                } 
            }
        }

        for (int i = 0; i < numThreads; i++) {
            workers.emplace_back([this]{chunkWorker();});
        }
    }

    void stopWorkers() {
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            stopThreads = true;
        }
        queueCond.notify_all();
        for (auto& worker : workers) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }

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

    uint8_t* getChunk(glm::vec3 position)
    {
        return voxelData[voxelChunkMapData[((int)position.z / 128) * 64 + ((int)position.y / 128) * 8 + ((int)position.x / 128)]].data;
    }

    uint16_t getChunkID(glm::vec3 position)
    {
        return voxelChunkMapData[((int)position.z / 128) * 64 + ((int)position.y / 128) * 8 + ((int)position.x / 128)];
    }

    glm::ivec3 rayCast(glm::vec3 rayOrigin, glm::vec3 rayDir, bool& something_hit, float maxDistance)
    {
        float tMax = 0, tMin = 0;

        glm::ivec3 origin = glm::ivec3(rayOrigin);

        uint8_t value = getChunk(origin)[((int)origin.z % 128) * 128*128 + ((int)origin.y % 128) * 128 + ((int)origin.x % 128)];
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

            value = getChunk(hit)[((int)hit.z % 128) * 128*128 + ((int)hit.y % 128) * 128 + ((int)hit.x % 128)];
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

                value = getChunk(hit)[((int)hit.z % 128) * 128*128 + ((int)hit.y % 128) * 128 + ((int)hit.x % 128)];
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


        // floor
        bool is_grounded = false;
        for (int x = -1; x < 2; x++)
        {
            for (int y = -1; y < 2; y++)
            {
                uint16_t chunkID = ((intCameraPosition.z + 15) / 128) * 64 + ((intCameraPosition.y + y) / 128) * 8 + ((intCameraPosition.x + x) / 128);
                is_grounded |= (MAT_HAS_COLLISION(voxel_mat(voxChunk(voxelData[voxelChunkMapData[chunkID]].data, intCameraPosition.x + x, intCameraPosition.y + y, intCameraPosition.z + 15)))); //MAT_HAS_COLLISION(voxel_mat(vox(intCameraPosition.x + x, intCameraPosition.y + y, intCameraPosition.z + 30)));
            }
        }

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
                    printf("\nMOVING");
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
                updateVoxelChunkMap(check.modValue, check.shift);
            } else if (check.pos > check.upperBound) {
                check.pos -= check.offset;
                updateVoxelChunkMap(check.modValue, check.modValue - check.shift);
            }
        }

        SortChunks();



        // Raycast and print distance
        if (windowManager->isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT))
        {
            bool hit = false;
            glm::ivec3 hitPosition = rayCast(-cameraPosition, cameraTargetPoint - cameraPosition, hit, 25.0f);
            if (hit)
            {   
                printf("HIT: %i, %i %i %i\n", hit, hitPosition.x, hitPosition.y, hitPosition.z);
                getChunk(hitPosition)[((int)hitPosition.z % 128) * 128*128 + ((int)hitPosition.y % 128) * 128 + ((int)hitPosition.x % 128)] = 0;

                for (int x = -4; x <= 4; x++)
                {
                    for (int y = -4; y <= 4; y++)
                    {
                        for (int z = -4; z <= 4; z++)
                        {
                            glm::ivec3 offset = glm::ivec3(x, y, z);
                            if (voxel_mat(getChunk(hitPosition + offset)[(((int)hitPosition.z + z) % 128) * 128*128 + (((int)hitPosition.y + y) % 128) * 128 + (((int)hitPosition.x + x) % 128)]) != MAT_AIR)
                            getChunk(hitPosition + offset)[(((int)hitPosition.z + z) % 128) * 128*128 + (((int)hitPosition.y + y) % 128) * 128 + (((int)hitPosition.x + x) % 128)] = 0;
                        }
                    }
                }
                
                uint16_t chunkID = getChunkID(hitPosition);
                
                chunkUpdateQueue[++chunkUpdateQueue[0]] = chunkID;
            }
        }
        
       


    }
    

    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels) {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = mipLevels;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (oldLayout == VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT; // The image is being read in shaders
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT; // The image will be written to during transfer

            sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; // Wait for fragment shader (or compute shader) to finish
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT; // Transfer operation will be the next stage
        } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT; // The image was written to during the transfer operation
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT; // The image will be read by shaders (for sampling)

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT; // Wait for the transfer operation to finish
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; // Fragment shader (or compute shader) will use the image
        } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = 0; // No access is required from the old layout
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT; // The image will be read by shaders (for sampling)

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT; // No need to wait for previous stages since it's undefined
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; // The image will be used in a fragment shader
        } else if (oldLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR && newLayout == VK_IMAGE_LAYOUT_GENERAL) {
            barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            destinationStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_GENERAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else {
            throw std::invalid_argument("unsupported layout transition!");
        }
        
        vkCmdPipelineBarrier(
            commandBuffer,
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );
        
        endSingleTimeCommands(commandBuffer);
    }

    void transitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels) {

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = mipLevels;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;
        if (oldLayout == VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT; // The image is being read in shaders
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT; // The image will be written to during transfer

            sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; // Wait for fragment shader (or compute shader) to finish
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT; // Transfer operation will be the next stage
        } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT; // The image was written to during the transfer operation
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT; // The image will be read by shaders (for sampling)

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT; // Wait for the transfer operation to finish
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; // Fragment shader (or compute shader) will use the image
        } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = 0; // No access is required from the old layout
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT; // The image will be read by shaders (for sampling)

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT; // No need to wait for previous stages since it's undefined
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; // The image will be used in a fragment shader
        } else if (oldLayout == VK_IMAGE_LAYOUT_GENERAL && newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
            barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR && newLayout == VK_IMAGE_LAYOUT_GENERAL) {
            barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            destinationStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_GENERAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else {
            throw std::invalid_argument("unsupported layout transition!");
        }

        vkCmdPipelineBarrier(
            commandBuffer,
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );
    }

    VkShaderModule createShaderModule(const std::vector<char>& code) {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(vulkanContext->getDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shader module!");
        }

        return shaderModule;
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

    static std::vector<char> readFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            std::cout << filename << std::endl;
            throw std::runtime_error("failed to open file!");
        }

        size_t fileSize = (size_t) file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
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