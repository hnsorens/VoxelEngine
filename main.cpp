// #define GLFW_INCLUDE_VULKAN
// #include <GLFW/glfw3.h>

// #include <iostream>
// #include <fstream>
// #include <stdexcept>
// #include <algorithm>
// #include <vector>
// #include <cstring>
// #include <cstdlib>
// #include <cstdint>
// #include <limits>
// #include <optional>
// #include <set>

// #define FNL_IMPL
// #include "FastNoiseLite.hpp"

// #define GLM_FORCE_RADIANS
// #define GLM_FORCE_DEPTH_ZERO_TO_ONCe
// #include <glm/glm.hpp>
// #include <glm/gtc/matrix_transform.hpp>

// const uint32_t WIDTH = 1920;
// const uint32_t HEIGHT = 1280;

// struct TransformUBO
// {
//     glm::mat4 view;
//     glm::mat4 proj;
// };

// const int MAX_FRAMES_IN_FLIGHT = 2;

// const std::vector<const char*> validationLayers = {
//     "VK_LAYER_KHRONOS_validation"
// };

// const std::vector<const char*> deviceExtensions = {
//     VK_KHR_SWAPCHAIN_EXTENSION_NAME,
//     VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
//     VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
//     VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
// };

// #ifdef NDEBUG
// const bool enableValidationLayers = true;
// #else
// const bool enableValidationLayers = true;
// #endif

// VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
//     auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
//     if (func != nullptr) {
//         return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
//     } else {
//         return VK_ERROR_EXTENSION_NOT_PRESENT;
//     }
// }

// void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
//     auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
//     if (func != nullptr) {
//         func(instance, debugMessenger, pAllocator);
//     }
// }

// struct QueueFamilyIndices {
//     std::optional<uint32_t> graphicsFamily;
//     std::optional<uint32_t> presentFamily;

//     bool isComplete() {
//         return graphicsFamily.has_value() && presentFamily.has_value();
//     }
// };

// struct SwapChainSupportDetails {
//     VkSurfaceCapabilitiesKHR capabilities;
//     std::vector<VkSurfaceFormatKHR> formats;
//     std::vector<VkPresentModeKHR> presentModes;
// };

// class VoxelEngine {
// public:
//     void run() {
//         initWindow();
//         initVulkan();
//         mainLoop();
//         cleanup();
//     }

// private:
//     GLFWwindow* window;

//     VkInstance instance;
//     VkDebugUtilsMessengerEXT debugMessenger;
//     VkSurfaceKHR surface;

//     VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
//     VkDevice device;

//     VkQueue graphicsQueue;
//     VkQueue presentQueue;

//     VkSwapchainKHR swapChain;
//     std::vector<VkImage> swapChainImages;
//     VkFormat swapChainImageFormat;
//     VkExtent2D swapChainExtent;
//     std::vector<VkImageView> swapChainImageViews;
//     std::vector<VkFramebuffer> swapChainFramebuffers;

//     VkRenderPass renderPass;
//     VkPipelineLayout pipelineLayout;
//     VkPipeline graphicsPipeline;

//     VkCommandPool commandPool;
//     std::vector<VkCommandBuffer> commandBuffers;
//     std::vector<VkCommandBuffer> raytracingCommandBuffers;

//     std::vector<VkCommandBuffer> commandBufferPairs;

//     std::vector<VkSemaphore> imageAvailableSemaphores;
//     std::vector<VkSemaphore> renderFinishedSemaphores;
//     std::vector<VkFence> inFlightFences;
//     uint32_t currentFrame = 0;
//     uint8_t section = 0;

//     // Raytracing
//     VkPipeline raytracingPipeline;
//     VkPipelineLayout raytracingPipelineLayout;
//     VkDescriptorSetLayout raytracingDescriptorSetLayout;
//     VkDescriptorPool raytracingDescriptorPool;

//     VkDescriptorSetLayout descriptorSetLayout;
//     VkDescriptorPool descriptorPool;

//     std::vector<VkDescriptorSet> raytracingDescriptorSets;
//     std::vector<VkDescriptorSet> descriptorSets;

//     VkStridedDeviceAddressRegionKHR raygenRegion{};
//     VkStridedDeviceAddressRegionKHR missRegion{};
//     VkStridedDeviceAddressRegionKHR hitRegion{};
//     VkStridedDeviceAddressRegionKHR callableRegion{};
//     VkDeviceSize sbtSize;
//     VkBuffer sbtBuffer;
//     VkDeviceMemory sbtMemory;

//     std::vector<VkImage> raytracingStorageImage;
//     std::vector<VkImageView> raytracingStorageImageView;

//     std::vector<VkSampler> imageSampler;

//     VkBuffer uniformBuffer;
//     VkDeviceMemory uniformBufferMemory;
//     TransformUBO ubo{};

//     void* uniformBuffersMapped;

//     // Camera

//     glm::vec3 cameraTargetPoint = glm::vec3(1.0f, 0.0f, 0.0f);
//     glm::vec3 cameraPosition = glm::vec3(-512.0f, -512.0f, -512.0f);

//     glm::vec3 cameraVelocity = glm::vec3(0.0f, 0.0f, 0.0f);

//     float pitch = 0.0f;  // Rotation around the X-axis (up/down)
//     float yaw = -90.0f;  // Rotation around the Y-axis (left/right)
//     float lastMouseX = 0.0f;
//     float lastMouseY = 0.0f;

//     float mouseX = 0.0f, mouseY = 0.0f;

//     float movementSpeed = 7.0f;
//     float mouseSensitivity = 0.1f;

//     bool framebufferResized = false;

//     // Voxels

//     VkImage voxelTexture;
//     VkDeviceMemory voxelTexturesMemory;
//     VkImageView voxelImageView;
//     VkSampler voxelTextureSampler;

//     VkBuffer voxelStagingBuffer;
//     VkDeviceMemory voxelStagingBufferMemory;


//     VkImage voxelUpperTexture;
//     VkDeviceMemory voxelUpperTexturesMemory;
//     VkImageView voxelUpperImageView;

//     VkBuffer voxelUpperStagingBuffer;
//     VkDeviceMemory voxelUpperStagingBufferMemory;



//     VkImage voxelChunkMapTexture;
//     VkDeviceMemory voxelChunkMapTexturesMemory;
//     VkImageView voxelChunkMapImageView;

//     VkBuffer voxelChunkMapStagingBuffer;
//     VkDeviceMemory voxelChunkMapStagingBufferMemory;

//     uint8_t* voxelData;
//     uint8_t* voxelUpperData;
//     uint16_t* voxelChunkMapData;

//     FastNoiseLite noise;

//     std::vector<VkBuffer> chunkComputeBufferPool;
//     std::vector<VkDeviceMemory> chunkComputerBufferMemoryPool;
//     std::vector<uint8_t*> chunkComputerBufferMappedMemoryPool;

//     VkPipeline chunkComputePipeline;
//     VkDescriptorSetLayout chunkComputeDescriptorSetLayout;
//     VkPipelineLayout chunkComputePipelineLayout;
//     VkDescriptorPool chunkComputeDescriptorPool;
//     std::vector<VkDescriptorSet> chunkComputeDescriptorSets;

//     std::vector<VkCommandBuffer> chunkComputeCommandBuffers;

//     std::vector<VkFence> chunkComputeFence;

//     double deltaTime = 0;
//     double lastTime = 0;

//     void initWindow() {
//         glfwInit();

//         glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

//         window = glfwCreateWindow(WIDTH, HEIGHT, "Voxels", nullptr, nullptr);
//         glfwSetWindowUserPointer(window, this);
//         glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
//     }

//     static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
//         auto app = reinterpret_cast<VoxelEngine*>(glfwGetWindowUserPointer(window));
//         app->framebufferResized = true;
//     }

//     void initVulkan() {
//         createInstance();
//         setupDebugMessenger();
//         createSurface();
//         pickPhysicalDevice();
//         createLogicalDevice();
//         createSwapChain();
//         CreateTransformUBO();
//         createImageViews();
//         createRenderPass();
//         createCommandPool();
//         createCommandBuffers();
//         createRaytracingStorageImage();
//         createGraphicsPipeline();
//         createFramebuffers();
//         createSyncObjects();

//         createChunkComputeBuffers();
//         createChunkComputePipeline();

//         createVoxelResources();
//         createRaytracingPipeline();
//         createRaytracingRegions();

        
//         for (int z = 0; z < 1; z++)
//         {
//             for (int y = 0; y < 1; y++)
//             {
//                 for (int x = 0; x < 8; x++)
//                 {
//                     generateChunk(x, glm::vec3(0.f, 0.f ,0.f));
//                 }

//                 for (int x = 0; x < 8; x++)
//                 {
//                     vkWaitForFences(device, 1, &chunkComputeFence[x], VK_TRUE, UINT64_MAX);
//                     uint8_t* data;
//                     vkMapMemory(device, chunkComputerBufferMemoryPool[x], 0, 128*128*128, 0, (void**)&data);

//                     for (int i = 0; i < 128; i++)
//                     {
//                         for (int i2 = 0; i2 < 128; i2++)
//                         {
//                             memcpy((char*)voxelData + ((z*128+i)*1024*1024 + (y*128+i2)*1024 + (x*128)), data + (i*128*128) + (i2*128), 128);
//                         }   
//                     }
                    

//                     vkUnmapMemory(device, chunkComputerBufferMemoryPool[x]);
//                 }
//             }
//             printf("Progress: %f\n", z / 8.0f);
//         }


//         // uint8_t* data;
//         // vkMapMemory(device, chunkComputerBufferMemoryPool[0], 0, 128*128*128, 0, (void**)&data);
        
//         // for (int i = 0; i < 128*128*128; i++)
//         // {
//         //     printf("%i\n", data[i]);
//         // }

//         // vkUnmapMemory(device, chunkComputerBufferMemoryPool[0]);
//     }

//     void mainLoop() {
//         while (!glfwWindowShouldClose(window)) {
//             deltaTime = glfwGetTime() - lastTime;
//             lastTime = glfwGetTime();
//             glfwPollEvents();
//             UpdateUBO();
//             drawFrame();
//         }

//         vkDeviceWaitIdle(device);
//     }

//     void cleanupSwapChain() {
//         for (auto framebuffer : swapChainFramebuffers) {
//             vkDestroyFramebuffer(device, framebuffer, nullptr);
//         }

//         for (auto imageView : swapChainImageViews) {
//             vkDestroyImageView(device, imageView, nullptr);
//         }

//         vkDestroySwapchainKHR(device, swapChain, nullptr);
//     }

//     void cleanup() {
//         cleanupSwapChain();

//         vkDestroyPipeline(device, graphicsPipeline, nullptr);
//         vkDestroyPipelineLayout(device, pipelineLayout, nullptr);

//         vkDestroyRenderPass(device, renderPass, nullptr);

//         for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
//             vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
//             vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
//             vkDestroyFence(device, inFlightFences[i], nullptr);
//         }

//         vkDestroyCommandPool(device, commandPool, nullptr);

//         vkDestroyDevice(device, nullptr);

//         if (enableValidationLayers) {
//             DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
//         }

//         vkDestroySurfaceKHR(instance, surface, nullptr);
//         vkDestroyInstance(instance, nullptr);

//         glfwDestroyWindow(window);

//         glfwTerminate();
//     }

//     void recreateSwapChain() {
//         int width = 0, height = 0;
//         glfwGetFramebufferSize(window, &width, &height);
//         while (width == 0 || height == 0) {
//             glfwGetFramebufferSize(window, &width, &height);
//             glfwWaitEvents();
//         }

//         vkDeviceWaitIdle(device);

//         cleanupSwapChain();

//         createSwapChain();
//         createImageViews();
//         createFramebuffers();
//         createRaytracingStorageImage();
//     }

//     void createInstance() {
//         if (enableValidationLayers && !checkValidationLayerSupport()) {
//             throw std::runtime_error("validation layers requested, but not available!");
//         }

//         VkApplicationInfo appInfo{};
//         appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
//         appInfo.pApplicationName = "Voxels";
//         appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
//         appInfo.pEngineName = "No Engine";
//         appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
//         appInfo.apiVersion = VK_API_VERSION_1_2;

//         VkInstanceCreateInfo createInfo{};
//         createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
//         createInfo.pApplicationInfo = &appInfo;

//         auto extensions = getRequiredExtensions();
//         createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
//         createInfo.ppEnabledExtensionNames = extensions.data();

//         VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
//         if (enableValidationLayers) {
//             createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
//             createInfo.ppEnabledLayerNames = validationLayers.data();

//             populateDebugMessengerCreateInfo(debugCreateInfo);
//             createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
//         } else {
//             createInfo.enabledLayerCount = 0;

//             createInfo.pNext = nullptr;
//         }

//         if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
//             throw std::runtime_error("failed to create instance!");
//         }
//     }

//     void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
//         createInfo = {};
//         createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
//         createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
//         createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
//         createInfo.pfnUserCallback = debugCallback;
//     }

//     void setupDebugMessenger() {
//         if (!enableValidationLayers) return;

//         VkDebugUtilsMessengerCreateInfoEXT createInfo;
//         populateDebugMessengerCreateInfo(createInfo);

//         if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
//             throw std::runtime_error("failed to set up debug messenger!");
//         }
//     }

//     void createSurface() {
//         if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
//             throw std::runtime_error("failed to create window surface!");
//         }
//     }

//     void pickPhysicalDevice() {
//         uint32_t deviceCount = 0;
//         vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

//         if (deviceCount == 0) {
//             throw std::runtime_error("failed to find GPUs with Vulkan support!");
//         }

//         std::vector<VkPhysicalDevice> devices(deviceCount);
//         vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

//         for (const auto& device : devices) {
//             if (isDeviceSuitable(device)) {
//                 physicalDevice = device;
//                 break;
//             }
//         }

//         if (physicalDevice == VK_NULL_HANDLE) {
//             throw std::runtime_error("failed to find a suitable GPU!");
//         }
//     }

//     void createLogicalDevice() {
//         QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

//         std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
//         std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

//         float queuePriority = 1.0f;
//         for (uint32_t queueFamily : uniqueQueueFamilies) {
//             VkDeviceQueueCreateInfo queueCreateInfo{};
//             queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
//             queueCreateInfo.queueFamilyIndex = queueFamily;
//             queueCreateInfo.queueCount = 1;
//             queueCreateInfo.pQueuePriorities = &queuePriority;
//             queueCreateInfos.push_back(queueCreateInfo);
//         }


//         // Enabling Raytracing Features

//         VkPhysicalDeviceBufferAddressFeaturesEXT bufferDeviceFeatures = {};
//         bufferDeviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_ADDRESS_FEATURES_EXT;
//         bufferDeviceFeatures.bufferDeviceAddress = VK_TRUE;
//         bufferDeviceFeatures.pNext = nullptr;

//         VkPhysicalDeviceDescriptorIndexingFeaturesEXT descriptorIndexingFeatures = {};
//         descriptorIndexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT;
//         descriptorIndexingFeatures.descriptorBindingPartiallyBound = VK_TRUE;
//         descriptorIndexingFeatures.descriptorBindingVariableDescriptorCount = VK_TRUE;
//         descriptorIndexingFeatures.pNext = &bufferDeviceFeatures;

//         VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingFeatures = {};
//         rayTracingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
//         rayTracingFeatures.rayTracingPipeline = VK_TRUE;
//         rayTracingFeatures.pNext = &descriptorIndexingFeatures;

//         VkPhysicalDeviceAccelerationStructureFeaturesKHR accelStructFeatures = {};
//         accelStructFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
//         accelStructFeatures.accelerationStructure = VK_TRUE;
//         accelStructFeatures.pNext = &rayTracingFeatures;

//         VkPhysicalDeviceFeatures2 deviceFeatures2 = {};
//         deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
//         deviceFeatures2.pNext = &accelStructFeatures;



//         VkDeviceCreateInfo createInfo{};
//         createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
//         createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
//         createInfo.pQueueCreateInfos = queueCreateInfos.data();

//         createInfo.pEnabledFeatures = nullptr;

//         createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
//         createInfo.ppEnabledExtensionNames = deviceExtensions.data();
//         createInfo.pNext = &deviceFeatures2;

//         if (enableValidationLayers) {
//             createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
//             createInfo.ppEnabledLayerNames = validationLayers.data();
//         } else {
//             createInfo.enabledLayerCount = 0;
//         }

//         if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
//             throw std::runtime_error("failed to create logical device!");
//         }

//         vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
//         vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
//     }

//     void createSwapChain() {
//         SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

//         VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
//         VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
//         VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

//         uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
//         if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
//             imageCount = swapChainSupport.capabilities.maxImageCount;
//         }

//         VkSwapchainCreateInfoKHR createInfo{};
//         createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
//         createInfo.surface = surface;

//         createInfo.minImageCount = imageCount;
//         createInfo.imageFormat = surfaceFormat.format;
//         createInfo.imageColorSpace = surfaceFormat.colorSpace;
//         createInfo.imageExtent = extent;
//         createInfo.imageArrayLayers = 1;
//         createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

//         QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
//         uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

//         if (indices.graphicsFamily != indices.presentFamily) {
//             createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
//             createInfo.queueFamilyIndexCount = 2;
//             createInfo.pQueueFamilyIndices = queueFamilyIndices;
//         } else {
//             createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
//         }

//         createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
//         createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
//         createInfo.presentMode = presentMode;
//         createInfo.clipped = VK_TRUE;

//         if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
//             throw std::runtime_error("failed to create swap chain!");
//         }

//         vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
//         swapChainImages.resize(imageCount);
//         vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

//         swapChainImageFormat = surfaceFormat.format;
//         swapChainExtent = extent;
//     }

//     void createImageViews() {
//         swapChainImageViews.resize(swapChainImages.size());

//         for (size_t i = 0; i < swapChainImages.size(); i++) {
//             VkImageViewCreateInfo createInfo{};
//             createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
//             createInfo.image = swapChainImages[i];
//             createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
//             createInfo.format = swapChainImageFormat;
//             createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
//             createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
//             createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
//             createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
//             createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//             createInfo.subresourceRange.baseMipLevel = 0;
//             createInfo.subresourceRange.levelCount = 1;
//             createInfo.subresourceRange.baseArrayLayer = 0;
//             createInfo.subresourceRange.layerCount = 1;

//             if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
//                 throw std::runtime_error("failed to create image views!");
//             }
//         }
//     }

//     void createRenderPass() {
//         VkAttachmentDescription colorAttachment{};
//         colorAttachment.format = swapChainImageFormat;
//         colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
//         colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
//         colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
//         colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
//         colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
//         colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//         colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

//         VkAttachmentReference colorAttachmentRef{};
//         colorAttachmentRef.attachment = 0;
//         colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

//         VkSubpassDescription subpass{};
//         subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
//         subpass.colorAttachmentCount = 1;
//         subpass.pColorAttachments = &colorAttachmentRef;

//         VkSubpassDependency dependency{};
//         dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
//         dependency.dstSubpass = 0;
//         dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//         dependency.srcAccessMask = 0;
//         dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//         dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

//         VkRenderPassCreateInfo renderPassInfo{};
//         renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
//         renderPassInfo.attachmentCount = 1;
//         renderPassInfo.pAttachments = &colorAttachment;
//         renderPassInfo.subpassCount = 1;
//         renderPassInfo.pSubpasses = &subpass;
//         renderPassInfo.dependencyCount = 1;
//         renderPassInfo.pDependencies = &dependency;

//         if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
//             throw std::runtime_error("failed to create render pass!");
//         }
//     }

// #define ALIGN_UP(value, alignment) (((value) + (alignment) - 1) & ~((alignment) - 1))


//     void createBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
//         VkBufferCreateInfo bufferInfo{};
//         bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
//         bufferInfo.size = size;
//         bufferInfo.usage = usage;
//         bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

//         if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
//             throw std::runtime_error("failed to create buffer!");
//         }

//         VkMemoryRequirements memRequirements;
//         vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

//         VkMemoryAllocateInfo allocInfo{};
//         allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
//         allocInfo.allocationSize = memRequirements.size;
//         allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);

//         if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
//             throw std::runtime_error("failed to allocate buffer memory!");
//         }

//         vkBindBufferMemory(device, buffer, bufferMemory, 0);
//     }


//     void createRaytracingRegions()
//     {
//         VkPhysicalDeviceProperties2 deviceProperties2 = {};
//         deviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
//         VkPhysicalDeviceRayTracingPipelinePropertiesKHR raytracingPipelineProperties = {};
//         raytracingPipelineProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
//         deviceProperties2.pNext = &raytracingPipelineProperties;
//         vkGetPhysicalDeviceProperties2(physicalDevice, &deviceProperties2);

//         VkDeviceSize handleSize = raytracingPipelineProperties.shaderGroupHandleSize;
//         VkDeviceSize handleSizeAligned = ALIGN_UP(handleSize, raytracingPipelineProperties.shaderGroupBaseAlignment);
//         sbtSize = handleSizeAligned * 2;

//         createBuffer(device, physicalDevice, sbtSize, VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
//             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, sbtBuffer, sbtMemory);


//         std::vector<uint8_t> shaderHandleStorage(sbtSize);

//         PFN_vkGetRayTracingShaderGroupHandlesKHR vkGetRayTracingShaderGroupHandlesKHR = reinterpret_cast<PFN_vkGetRayTracingShaderGroupHandlesKHR>(vkGetDeviceProcAddr(device, "vkGetRayTracingShaderGroupHandlesKHR"));
//         vkGetRayTracingShaderGroupHandlesKHR(device, raytracingPipeline, 0, 2, sbtSize, shaderHandleStorage.data());

//         void* mappedData;
//         vkMapMemory(device, sbtMemory, 0, sbtSize, 0, &mappedData);
//         memcpy(mappedData, shaderHandleStorage.data(), sbtSize);
//         vkUnmapMemory(device, sbtMemory);


//         VkBufferDeviceAddressInfo bufferAddressInfo = {};
//         bufferAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
//         bufferAddressInfo.buffer = sbtBuffer;
//         VkDeviceAddress sbtAddress = vkGetBufferDeviceAddress(device, &bufferAddressInfo);

//         raygenRegion.deviceAddress = sbtAddress;
//         raygenRegion.stride = handleSizeAligned;
//         raygenRegion.size = handleSizeAligned;

//         missRegion.deviceAddress = sbtAddress + handleSizeAligned;
//         missRegion.stride = handleSizeAligned;
//         missRegion.size = handleSizeAligned;

//         hitRegion = {};
//         callableRegion = {};   
//     }

//     void createRaytracingStorageImage()
//     {
//         raytracingStorageImage.resize(MAX_FRAMES_IN_FLIGHT);
//         raytracingStorageImageView.resize(MAX_FRAMES_IN_FLIGHT);
        
//         for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
//         {
//             VkImageCreateInfo imageCreateInfo = {};
//             imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
//             imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
//             imageCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
//             imageCreateInfo.extent.width = swapChainExtent.width;
//             imageCreateInfo.extent.height = swapChainExtent.height;
//             imageCreateInfo.extent.depth = 1;
//             imageCreateInfo.mipLevels = 1;
//             imageCreateInfo.arrayLayers = 1;
//             imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
//             imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
//             imageCreateInfo.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
//             imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
//             imageCreateInfo.queueFamilyIndexCount = 0;
//             imageCreateInfo.pQueueFamilyIndices = nullptr;
//             imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

//             if (vkCreateImage(device, &imageCreateInfo, nullptr, &raytracingStorageImage[i]) != VK_SUCCESS)
//             {
//                 throw std::runtime_error("failed to create raytracing storage image!");
//             }

//             VkMemoryRequirements memRequirements;
//             vkGetImageMemoryRequirements(device, raytracingStorageImage[i], &memRequirements);

//             VkMemoryAllocateInfo allocInfo = {};
//             allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
//             allocInfo.allocationSize = memRequirements.size;

//             VkPhysicalDeviceMemoryProperties memProperties;
//             vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

//             for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
//             {
//                 if ((memRequirements.memoryTypeBits & (1 << i)) &&
//                 (memProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
//                 {
//                     allocInfo.memoryTypeIndex = 1;
//                     break;
//                 }
//             }

//             VkDeviceMemory imageMemory;
//             if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
//             {
//                 throw std::runtime_error("Failed to allocate memory for image!");
//             }

//             vkBindImageMemory(device, raytracingStorageImage[i], imageMemory, 0);

//             // Create Image View


//             VkImageViewCreateInfo viewCreateInfo = {};
//             viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
//             viewCreateInfo.image = raytracingStorageImage[i];
//             viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
//             viewCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
//             viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//             viewCreateInfo.subresourceRange.baseMipLevel = 0;
//             viewCreateInfo.subresourceRange.levelCount = 1;
//             viewCreateInfo.subresourceRange.baseArrayLayer = 0;
//             viewCreateInfo.subresourceRange.layerCount = 1;

//             if (vkCreateImageView(device, &viewCreateInfo, nullptr, &raytracingStorageImageView[i]) != VK_SUCCESS)
//             {
//                 throw std::runtime_error("failed to creaet raytracing image view!");
//             }

//             transitionImageLayout(raytracingStorageImage[i], VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, 1);
//         }
        
//     }

//     void createVoxelResources()
//     {
//         // Create the image (texture)
//         VkImageCreateInfo imageInfo = {};
//         imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
//         imageInfo.imageType = VK_IMAGE_TYPE_3D;
//         imageInfo.extent.width = 1024;
//         imageInfo.extent.height = 1024;
//         imageInfo.extent.depth = 1024;
//         imageInfo.mipLevels = 1;
//         imageInfo.arrayLayers = 1;
//         imageInfo.format = VK_FORMAT_R8_UINT;
//         imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
//         imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//         imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
//         imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
//         imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

//         if (vkCreateImage(device, &imageInfo, nullptr, &voxelTexture) != VK_SUCCESS) {
//             throw std::runtime_error("failed to create texture image!");
//         }

//         // Allocate memory for the image
//         VkMemoryRequirements memRequirements;
//         vkGetImageMemoryRequirements(device, voxelTexture, &memRequirements);

//         VkMemoryAllocateInfo allocInfo = {};
//         allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
//         allocInfo.allocationSize = memRequirements.size;
//         allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

//         if (vkAllocateMemory(device, &allocInfo, nullptr, &voxelTexturesMemory) != VK_SUCCESS) {
//             throw std::runtime_error("failed to allocate memory for texture image!");
//         }

//         vkBindImageMemory(device, voxelTexture, voxelTexturesMemory, 0);

//         VkImageViewCreateInfo viewInfo = {};
//         viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
//         viewInfo.image = voxelTexture;
//         viewInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
//         viewInfo.format = VK_FORMAT_R8_UINT;
//         viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
//         viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
//         viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
//         viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
//         viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//         viewInfo.subresourceRange.baseMipLevel = 0;
//         viewInfo.subresourceRange.levelCount = 1;
//         viewInfo.subresourceRange.baseArrayLayer = 0;
//         viewInfo.subresourceRange.layerCount = 1;

//         if (vkCreateImageView(device, &viewInfo, nullptr, &voxelImageView) != VK_SUCCESS) {
//             throw std::runtime_error("failed to create texture image view!");
//         }

//         VkSamplerCreateInfo samplerCreateInfo = {};
//         samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
//         samplerCreateInfo.magFilter = VK_FILTER_NEAREST;
//         samplerCreateInfo.minFilter = VK_FILTER_NEAREST;
//         samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
//         samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
//         samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
//         samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
//         samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
//         samplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;

//         vkCreateSampler(device, &samplerCreateInfo, nullptr, &voxelTextureSampler);


//         // Staging buffer to update the texture

//         VkBufferCreateInfo bufferCreateInfo = {};
//         bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
//         bufferCreateInfo.size = 1024*1024*1024;
//         bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
//         bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

//         if (vkCreateBuffer(device, &bufferCreateInfo, nullptr, &voxelStagingBuffer) != VK_SUCCESS)
//         {
//             throw std::runtime_error("Failed to create voxel staging buffer");
//         }

//         VkMemoryRequirements stagingMemRequirements;
//         vkGetBufferMemoryRequirements(device, voxelStagingBuffer, &stagingMemRequirements);

//         VkMemoryAllocateInfo stagingAllocInfo = {};
//         stagingAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
//         stagingAllocInfo.allocationSize = stagingMemRequirements.size;
//         stagingAllocInfo.memoryTypeIndex = findMemoryType(physicalDevice, stagingMemRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

//         vkAllocateMemory(device, &stagingAllocInfo, nullptr, &voxelStagingBufferMemory);
//         vkBindBufferMemory(device, voxelStagingBuffer, voxelStagingBufferMemory, 0);
        
//         transitionImageLayout(voxelTexture, VK_FORMAT_R8_UINT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL, 1);

//         // Voxel Data

//         voxelData = new uint8_t[1024*1024*1024]();

//         // for (int i = 0; i < 1024*1024*1024; i++)
//         // {
//         //     voxelData[i] = 0b11100000;
//         // }

//         // for (int z = 0; z < 1024; z+=2)
//         // {
//         //     for (int y = 0; y < 1024; y+=2)
//         //     {
//         //         for (int x = 0; x < 1024; x+=2)
//         //         {
//         //             voxelData[z * 1024 * 1024 + y * 1024 + x] = 0b11100001;
//         //         }   
//         //     }   
//         // }

//         // for (int z = 0; z < 123; z++)
//         // {
//         //     for (int y = 0; y < 123; y++)
//         //     {
//         //         for (int x = 0; x < 123; x++)
//         //         {
//         //             float v = noise.GetNoise(x * 0.3f, y * 0.3f, z * 0.3f);
//         //             if (v > 0.1)
//         //             {
//         //                 voxelData[z * 1024 * 1024 + y * 1024 + x] = 0b11100000;
//         //             }
//         //             else
//         //             {
//         //                 voxelData[z * 1024 * 1024 + y * 1024 + x] = 0b11100001;
//         //             }
//         //         }   
//         //     }   
//         //     printf("Progress: %f\n", z / 1024.0f);
//         // }

//         // Upper
//         {


//         // Create the image (texture)
//         VkImageCreateInfo upperImageInfo = {};
//         upperImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
//         upperImageInfo.imageType = VK_IMAGE_TYPE_3D;
//         upperImageInfo.extent.width = 64;
//         upperImageInfo.extent.height = 64;
//         upperImageInfo.extent.depth = 64;
//         upperImageInfo.mipLevels = 1;
//         upperImageInfo.arrayLayers = 1;
//         upperImageInfo.format = VK_FORMAT_R8_UINT;
//         upperImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
//         upperImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//         upperImageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
//         upperImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
//         upperImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

//         if (vkCreateImage(device, &upperImageInfo, nullptr, &voxelUpperTexture) != VK_SUCCESS) {
//             throw std::runtime_error("failed to create texture image!");
//         }

//         // Allocate memory for the image
//         VkMemoryRequirements upperMemRequirements;
//         vkGetImageMemoryRequirements(device, voxelUpperTexture, &upperMemRequirements);

//         VkMemoryAllocateInfo upperAllocInfo = {};
//         upperAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
//         upperAllocInfo.allocationSize = upperMemRequirements.size;
//         upperAllocInfo.memoryTypeIndex = findMemoryType(physicalDevice, upperMemRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

//         if (vkAllocateMemory(device, &upperAllocInfo, nullptr, &voxelUpperTexturesMemory) != VK_SUCCESS) {
//             throw std::runtime_error("failed to allocate memory for texture image!");
//         }

//         vkBindImageMemory(device, voxelUpperTexture, voxelUpperTexturesMemory, 0);

//         VkImageViewCreateInfo upperViewInfo = {};
//         upperViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
//         upperViewInfo.image = voxelUpperTexture;
//         upperViewInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
//         upperViewInfo.format = VK_FORMAT_R8_UINT;
//         upperViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
//         upperViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
//         upperViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
//         upperViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
//         upperViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//         upperViewInfo.subresourceRange.baseMipLevel = 0;
//         upperViewInfo.subresourceRange.levelCount = 1;
//         upperViewInfo.subresourceRange.baseArrayLayer = 0;
//         upperViewInfo.subresourceRange.layerCount = 1;

//         if (vkCreateImageView(device, &upperViewInfo, nullptr, &voxelUpperImageView) != VK_SUCCESS) {
//             throw std::runtime_error("failed to create texture image view!");
//         }


//         // Staging buffer to update the texture

//         VkBufferCreateInfo upperBufferCreateInfo = {};
//         upperBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
//         upperBufferCreateInfo.size = 64*64*64;
//         upperBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
//         upperBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

//         if (vkCreateBuffer(device, &upperBufferCreateInfo, nullptr, &voxelUpperStagingBuffer) != VK_SUCCESS)
//         {
//             throw std::runtime_error("Failed to create voxel staging buffer");
//         }

//         VkMemoryRequirements upperStagingMemRequirements;
//         vkGetBufferMemoryRequirements(device, voxelUpperStagingBuffer, &upperStagingMemRequirements);

//         VkMemoryAllocateInfo upperStagingAllocInfo = {};
//         upperStagingAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
//         upperStagingAllocInfo.allocationSize = upperStagingMemRequirements.size;
//         upperStagingAllocInfo.memoryTypeIndex = findMemoryType(physicalDevice, upperStagingMemRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

//         vkAllocateMemory(device, &upperStagingAllocInfo, nullptr, &voxelUpperStagingBufferMemory);
//         vkBindBufferMemory(device, voxelUpperStagingBuffer, voxelUpperStagingBufferMemory, 0);
        
//         transitionImageLayout(voxelUpperTexture, VK_FORMAT_R8_UINT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL, 1);

//         // Voxel Data

//         voxelUpperData = new uint8_t[64*64*64]();

//     }



// {






//         // Create the image (texture)
//         VkImageCreateInfo voxelChunkMapImageInfo = {};
//         voxelChunkMapImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
//         voxelChunkMapImageInfo.imageType = VK_IMAGE_TYPE_3D;
//         voxelChunkMapImageInfo.extent.width = 8;
//         voxelChunkMapImageInfo.extent.height = 8;
//         voxelChunkMapImageInfo.extent.depth = 8;
//         voxelChunkMapImageInfo.mipLevels = 1;
//         voxelChunkMapImageInfo.arrayLayers = 1;
//         voxelChunkMapImageInfo.format = VK_FORMAT_R16_UINT;
//         voxelChunkMapImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
//         voxelChunkMapImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//         voxelChunkMapImageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
//         voxelChunkMapImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
//         voxelChunkMapImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

//         if (vkCreateImage(device, &voxelChunkMapImageInfo, nullptr, &voxelChunkMapTexture) != VK_SUCCESS) {
//             throw std::runtime_error("failed to create texture image!");
//         }

//         // Allocate memory for the image
//         VkMemoryRequirements voxelChunkMapMemRequirements;
//         vkGetImageMemoryRequirements(device, voxelChunkMapTexture, &voxelChunkMapMemRequirements);

//         VkMemoryAllocateInfo voxelChunkMapAllocInfo = {};
//         voxelChunkMapAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
//         voxelChunkMapAllocInfo.allocationSize = voxelChunkMapMemRequirements.size;
//         voxelChunkMapAllocInfo.memoryTypeIndex = findMemoryType(physicalDevice, voxelChunkMapMemRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

//         if (vkAllocateMemory(device, &voxelChunkMapAllocInfo, nullptr, &voxelChunkMapTexturesMemory) != VK_SUCCESS) {
//             throw std::runtime_error("failed to allocate memory for texture image!");
//         }

//         vkBindImageMemory(device, voxelChunkMapTexture, voxelChunkMapTexturesMemory, 0);

//         VkImageViewCreateInfo voxelChunkMapViewInfo = {};
//         voxelChunkMapViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
//         voxelChunkMapViewInfo.image = voxelChunkMapTexture;
//         voxelChunkMapViewInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
//         voxelChunkMapViewInfo.format = VK_FORMAT_R16_UINT;
//         voxelChunkMapViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
//         voxelChunkMapViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
//         voxelChunkMapViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
//         voxelChunkMapViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
//         voxelChunkMapViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//         voxelChunkMapViewInfo.subresourceRange.baseMipLevel = 0;
//         voxelChunkMapViewInfo.subresourceRange.levelCount = 1;
//         voxelChunkMapViewInfo.subresourceRange.baseArrayLayer = 0;
//         voxelChunkMapViewInfo.subresourceRange.layerCount = 1;

//         if (vkCreateImageView(device, &voxelChunkMapViewInfo, nullptr, &voxelChunkMapImageView) != VK_SUCCESS) {
//             throw std::runtime_error("failed to create texture image view!");
//         }


//         // Staging buffer to update the texture

//         VkBufferCreateInfo voxelChunkMapBufferCreateInfo = {};
//         voxelChunkMapBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
//         voxelChunkMapBufferCreateInfo.size = 8*8*8*2;
//         voxelChunkMapBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
//         voxelChunkMapBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

//         if (vkCreateBuffer(device, &voxelChunkMapBufferCreateInfo, nullptr, &voxelChunkMapStagingBuffer) != VK_SUCCESS)
//         {
//             throw std::runtime_error("Failed to create voxel staging buffer");
//         }

//         VkMemoryRequirements voxelChunkMapStagingMemRequirements;
//         vkGetBufferMemoryRequirements(device, voxelChunkMapStagingBuffer, &voxelChunkMapStagingMemRequirements);

//         VkMemoryAllocateInfo voxelChunkMapStagingAllocInfo = {};
//         voxelChunkMapStagingAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
//         voxelChunkMapStagingAllocInfo.allocationSize = voxelChunkMapStagingMemRequirements.size;
//         voxelChunkMapStagingAllocInfo.memoryTypeIndex = findMemoryType(physicalDevice, voxelChunkMapStagingMemRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

//         vkAllocateMemory(device, &voxelChunkMapStagingAllocInfo, nullptr, &voxelChunkMapStagingBufferMemory);
//         vkBindBufferMemory(device, voxelChunkMapStagingBuffer, voxelChunkMapStagingBufferMemory, 0);
        
//         transitionImageLayout(voxelChunkMapTexture, VK_FORMAT_R16_UINT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL, 1);

//         // Voxel Data

//         voxelChunkMapData = new uint16_t[8*8*8]();

//         for (int i = 0; i < 8*8*8; i++)
//         {
//             voxelChunkMapData[i] = i;
//         }
// }
//     }

//     void UpdateVoxels(VkCommandBuffer commandBuffer, void* data, uint8_t section)
//     {
    


//         void* mappedData;
//         vkMapMemory(device, voxelStagingBufferMemory, 0, 1024*1024*128, 0, &mappedData);
//         memcpy(mappedData, (char*)voxelData + (section * 1024*1024*128), 1024*1024*128);
//         vkUnmapMemory(device, voxelStagingBufferMemory);

//         VkBufferImageCopy region = {};
//         region.bufferOffset = 0;
//         region.bufferRowLength = 0;
//         region.bufferImageHeight = 0;
//         region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//         region.imageSubresource.mipLevel = 0;
//         region.imageSubresource.baseArrayLayer = 0;
//         region.imageSubresource.layerCount = 1;
//         region.imageOffset = { 0, 0, 128 * section };
//         region.imageExtent = { 1024, 1024, 128 };

//         vkCmdCopyBufferToImage(commandBuffer, voxelStagingBuffer, voxelTexture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);


//         void* mappedUpperData;
//         vkMapMemory(device, voxelUpperStagingBufferMemory, 0, 32 * 32 * 8, 0, &mappedUpperData);
//         memcpy(mappedUpperData, (char*)voxelUpperData + (section * 32 * 32 * 8), 32*32*4);
//         vkUnmapMemory(device, voxelUpperStagingBufferMemory);

//         VkBufferImageCopy upperRegion = {};
//         upperRegion.bufferOffset = 0;
//         upperRegion.bufferRowLength = 0;
//         upperRegion.bufferImageHeight = 0;
//         upperRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//         upperRegion.imageSubresource.mipLevel = 0;
//         upperRegion.imageSubresource.baseArrayLayer = 0;
//         upperRegion.imageSubresource.layerCount = 1;
//         upperRegion.imageOffset = { 0,0, section*4 };
//         upperRegion.imageExtent = { 32,32,32 };

//         vkCmdCopyBufferToImage(commandBuffer, voxelUpperStagingBuffer, voxelUpperTexture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &upperRegion);



//         void* mappedVoxelChunkMapData;
//         vkMapMemory(device, voxelChunkMapStagingBufferMemory, 0, 8*8*8*2, 0, &mappedVoxelChunkMapData);
//         memcpy(mappedVoxelChunkMapData, voxelChunkMapData, 8*8*8*2);
//         vkUnmapMemory(device, voxelChunkMapStagingBufferMemory);

//         VkBufferImageCopy voxelChunkMapRegion = {};
//         voxelChunkMapRegion.bufferOffset = 0;
//         voxelChunkMapRegion.bufferRowLength = 0;
//         voxelChunkMapRegion.bufferImageHeight = 0;
//         voxelChunkMapRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//         voxelChunkMapRegion.imageSubresource.mipLevel = 0;
//         voxelChunkMapRegion.imageSubresource.baseArrayLayer = 0;
//         voxelChunkMapRegion.imageSubresource.layerCount = 1;
//         voxelChunkMapRegion.imageOffset = { 0,0,0 };
//         voxelChunkMapRegion.imageExtent = { 8,8,8 };

//         vkCmdCopyBufferToImage(commandBuffer, voxelChunkMapStagingBuffer, voxelChunkMapTexture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &voxelChunkMapRegion);
//     }


//     void createRaytracingPipeline()
//     {
//         auto raygenShaderCode = readFile("../rgen.spv");
//         auto missShaderCode = readFile("../rmiss.spv");

//         VkShaderModule raygenShaderModule = createShaderModule(raygenShaderCode);
//         VkShaderModule missShaderModule = createShaderModule(missShaderCode);

//         VkPipelineShaderStageCreateInfo raygenShaderStageInfo{};
//         raygenShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
//         raygenShaderStageInfo.stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
//         raygenShaderStageInfo.module = raygenShaderModule;
//         raygenShaderStageInfo.pName = "main";

//         VkPipelineShaderStageCreateInfo missShaderStageInfo{};
//         missShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
//         missShaderStageInfo.stage = VK_SHADER_STAGE_MISS_BIT_KHR;
//         missShaderStageInfo.module = missShaderModule;
//         missShaderStageInfo.pName = "main";

//         VkPipelineShaderStageCreateInfo shaderStages[] = {raygenShaderStageInfo, missShaderStageInfo};

//         VkRayTracingShaderGroupCreateInfoKHR shaderGroups[2] = {};

//         shaderGroups[0].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
//         shaderGroups[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
//         shaderGroups[0].generalShader = 0;
//         shaderGroups[0].closestHitShader = VK_SHADER_UNUSED_KHR;
//         shaderGroups[0].anyHitShader = VK_SHADER_UNUSED_KHR;
//         shaderGroups[0].intersectionShader = VK_SHADER_UNUSED_KHR;

//         shaderGroups[1].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
//         shaderGroups[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
//         shaderGroups[1].generalShader = 1;
//         shaderGroups[1].closestHitShader = VK_SHADER_UNUSED_KHR;
//         shaderGroups[1].anyHitShader = VK_SHADER_UNUSED_KHR;
//         shaderGroups[1].intersectionShader = VK_SHADER_UNUSED_KHR;


//         VkDescriptorSetLayoutBinding storageImageBinding = {};
//         storageImageBinding.binding = 0;
//         storageImageBinding.descriptorCount = 1;
//         storageImageBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
//         storageImageBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
//         storageImageBinding.pImmutableSamplers = nullptr;

//         VkDescriptorSetLayoutBinding uniformBinding = {};
//         uniformBinding.binding = 1;
//         uniformBinding.descriptorCount = 1;
//         uniformBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//         uniformBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
//         uniformBinding.pImmutableSamplers = nullptr;

//         VkDescriptorSetLayoutBinding upperVoxelTexture = {};
//         upperVoxelTexture.binding = 2;
//         upperVoxelTexture.descriptorCount = 1;
//         upperVoxelTexture.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
//         upperVoxelTexture.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
//         upperVoxelTexture.pImmutableSamplers = nullptr;

//         VkDescriptorSetLayoutBinding voxelTexture = {};
//         voxelTexture.binding = 3;
//         voxelTexture.descriptorCount = 1;
//         voxelTexture.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
//         voxelTexture.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
//         voxelTexture.pImmutableSamplers = nullptr;

//         VkDescriptorSetLayoutBinding voxelChunkMapTexture = {};
//         voxelChunkMapTexture.binding = 4;
//         voxelChunkMapTexture.descriptorCount = 1;
//         voxelChunkMapTexture.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
//         voxelChunkMapTexture.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
//         voxelChunkMapTexture.pImmutableSamplers = nullptr;

//         VkDescriptorSetLayoutBinding bindings[] = {storageImageBinding, uniformBinding, voxelTexture, upperVoxelTexture, voxelChunkMapTexture};

//         VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {};
//         layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
//         layoutCreateInfo.bindingCount = 5;
//         layoutCreateInfo.pBindings = bindings;
//         layoutCreateInfo.pNext = nullptr;

//         if (vkCreateDescriptorSetLayout(device, &layoutCreateInfo, nullptr, &raytracingDescriptorSetLayout) != VK_SUCCESS)
//         {
//             throw std::runtime_error("failed to create raytracing descriptor set layout!");
//         }

//         VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
//         pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
//         pipelineLayoutCreateInfo.setLayoutCount = 1;
//         pipelineLayoutCreateInfo.pSetLayouts = &raytracingDescriptorSetLayout;

//         if (vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &raytracingPipelineLayout) != VK_SUCCESS)
//         {
//             throw std::runtime_error("failed to create raytracing pipeline layout!");
//         }

//         printf("PIPELINE LAYOUT %i\n", 1);

//         VkRayTracingPipelineCreateInfoKHR pipelineCreateInfo{};
//         pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
//         pipelineCreateInfo.stageCount = 2;
//         pipelineCreateInfo.pStages = shaderStages;
//         pipelineCreateInfo.groupCount = 2;
//         pipelineCreateInfo.pGroups = shaderGroups;
//         pipelineCreateInfo.maxPipelineRayRecursionDepth = 1;
//         pipelineCreateInfo.layout = raytracingPipelineLayout;

//         PFN_vkCreateRayTracingPipelinesKHR vkCreateRayTracingPipelinesKHR = reinterpret_cast<PFN_vkCreateRayTracingPipelinesKHR>(vkGetDeviceProcAddr(device, "vkCreateRayTracingPipelinesKHR"));
//         if (vkCreateRayTracingPipelinesKHR(device, VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &raytracingPipeline) != VK_SUCCESS)
//         {
//             throw std::runtime_error("Failed to create raytracing pipeline!");
//         }
//         printf("PIPELINE LAYOUT %i\n", 1);
//         vkDestroyShaderModule(device, raygenShaderModule, nullptr);
//         vkDestroyShaderModule(device, missShaderModule, nullptr);


//         // Pool

//         VkDescriptorPoolSize storageImagePoolSize = {};
//         storageImagePoolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
//         storageImagePoolSize.descriptorCount = 2;

//         VkDescriptorPoolSize uboPoolSize = {};
//         uboPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//         uboPoolSize.descriptorCount = 2;

//         VkDescriptorPoolSize upperVoxelPoolSize = {};
//         upperVoxelPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
//         upperVoxelPoolSize.descriptorCount = 6;

//         VkDescriptorPoolSize poolSizes[] = {storageImagePoolSize, uboPoolSize, upperVoxelPoolSize};

//         VkDescriptorPoolCreateInfo poolCreateInfo = {};
//         poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
//         poolCreateInfo.poolSizeCount = 3;
//         poolCreateInfo.pPoolSizes = poolSizes;
//         poolCreateInfo.maxSets = 50;

//         if (vkCreateDescriptorPool(device, &poolCreateInfo, nullptr, &raytracingDescriptorPool) != VK_SUCCESS)
//         {
//             throw std::runtime_error("failed to create raytracing descriptor pool!");
//         }

//         // Updating Descriptor Set

//         raytracingDescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);

//         VkDescriptorSetLayout setLayouts[] = {raytracingDescriptorSetLayout, raytracingDescriptorSetLayout};

//         VkDescriptorSetAllocateInfo allocInfo = {};
//         allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
//         allocInfo.descriptorPool = raytracingDescriptorPool;
//         allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
//         allocInfo.pSetLayouts = setLayouts;
        
//         if (vkAllocateDescriptorSets(device, &allocInfo, raytracingDescriptorSets.data()))
//         {
//             throw std::runtime_error("Failed to create raytracing descriptor set!");
//         }

//         for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
//         {
            

//             VkDescriptorImageInfo imageInfo = {};
//             imageInfo.imageView = raytracingStorageImageView[i];
//             imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

//             VkWriteDescriptorSet writeStorageDescriptorSet = {};
//             writeStorageDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//             writeStorageDescriptorSet.dstSet = raytracingDescriptorSets[i];
//             writeStorageDescriptorSet.dstBinding = 0;
//             writeStorageDescriptorSet.dstArrayElement = 0;
//             writeStorageDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
//             writeStorageDescriptorSet.descriptorCount = 1;
//             writeStorageDescriptorSet.pImageInfo = &imageInfo;

//             VkDescriptorBufferInfo bufferInfo{};
//             bufferInfo.buffer = uniformBuffer;
//             bufferInfo.offset = 0;
//             bufferInfo.range = sizeof(TransformUBO);

//             VkWriteDescriptorSet writeTransformDescriptorSet = {};
//             writeTransformDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//             writeTransformDescriptorSet.dstSet = raytracingDescriptorSets[i];
//             writeTransformDescriptorSet.dstBinding = 1;
//             writeTransformDescriptorSet.dstArrayElement = 0;
//             writeTransformDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//             writeTransformDescriptorSet.descriptorCount = 1;
//             writeTransformDescriptorSet.pBufferInfo = &bufferInfo;


            
//             VkDescriptorImageInfo upperVoxelImageInfo = {};
//             upperVoxelImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//             upperVoxelImageInfo.imageView = voxelUpperImageView;
//             upperVoxelImageInfo.sampler = voxelTextureSampler;

//             VkWriteDescriptorSet upperVoxelDescriptorSet = {};
//             upperVoxelDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//             upperVoxelDescriptorSet.dstSet = raytracingDescriptorSets[i];
//             upperVoxelDescriptorSet.dstBinding = 2;
//             upperVoxelDescriptorSet.dstArrayElement = 0;
//             upperVoxelDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
//             upperVoxelDescriptorSet.descriptorCount = 1;
//             upperVoxelDescriptorSet.pImageInfo = &upperVoxelImageInfo;


//             VkDescriptorImageInfo voxelImageInfo = {};
//             voxelImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//             voxelImageInfo.imageView = voxelImageView;
//             voxelImageInfo.sampler = voxelTextureSampler;

//             VkWriteDescriptorSet voxelDescriptorSet = {};
//             voxelDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//             voxelDescriptorSet.dstSet = raytracingDescriptorSets[i];
//             voxelDescriptorSet.dstBinding = 3;
//             voxelDescriptorSet.dstArrayElement = 0;
//             voxelDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
//             voxelDescriptorSet.descriptorCount = 1;
//             voxelDescriptorSet.pImageInfo = &voxelImageInfo;


//             VkDescriptorImageInfo voxelChunkMapImageInfo = {};
//             voxelChunkMapImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//             voxelChunkMapImageInfo.imageView = voxelChunkMapImageView;
//             voxelChunkMapImageInfo.sampler = voxelTextureSampler;

//             VkWriteDescriptorSet voxelChunkMapDescriptorSet = {};
//             voxelChunkMapDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//             voxelChunkMapDescriptorSet.dstSet = raytracingDescriptorSets[i];
//             voxelChunkMapDescriptorSet.dstBinding = 4;
//             voxelChunkMapDescriptorSet.dstArrayElement = 0;
//             voxelChunkMapDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
//             voxelChunkMapDescriptorSet.descriptorCount = 1;
//             voxelChunkMapDescriptorSet.pImageInfo = &voxelChunkMapImageInfo;

//             VkWriteDescriptorSet writeDescriptorSets[] = {writeStorageDescriptorSet, writeTransformDescriptorSet, upperVoxelDescriptorSet, voxelDescriptorSet, voxelChunkMapDescriptorSet};

//             vkUpdateDescriptorSets(device, 5, writeDescriptorSets, 0, nullptr);
//         }

//     }

//     void createGraphicsPipeline() {
//         auto vertShaderCode = readFile("../vert.spv");
//         auto fragShaderCode = readFile("../frag.spv");

//         VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
//         VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

//         VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
//         vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
//         vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
//         vertShaderStageInfo.module = vertShaderModule;
//         vertShaderStageInfo.pName = "main";

//         VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
//         fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
//         fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
//         fragShaderStageInfo.module = fragShaderModule;
//         fragShaderStageInfo.pName = "main";

//         VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

//         VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
//         vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
//         vertexInputInfo.vertexBindingDescriptionCount = 0;
//         vertexInputInfo.vertexAttributeDescriptionCount = 0;

//         VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
//         inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
//         inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
//         inputAssembly.primitiveRestartEnable = VK_FALSE;

//         VkPipelineViewportStateCreateInfo viewportState{};
//         viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
//         viewportState.viewportCount = 1;
//         viewportState.scissorCount = 1;

//         VkPipelineRasterizationStateCreateInfo rasterizer{};
//         rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
//         rasterizer.depthClampEnable = VK_FALSE;
//         rasterizer.rasterizerDiscardEnable = VK_FALSE;
//         rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
//         rasterizer.lineWidth = 1.0f;
//         rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
//         rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
//         rasterizer.depthBiasEnable = VK_FALSE;

//         VkPipelineMultisampleStateCreateInfo multisampling{};
//         multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
//         multisampling.sampleShadingEnable = VK_FALSE;
//         multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

//         VkPipelineColorBlendAttachmentState colorBlendAttachment{};
//         colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
//         colorBlendAttachment.blendEnable = VK_FALSE;

//         VkPipelineColorBlendStateCreateInfo colorBlending{};
//         colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
//         colorBlending.logicOpEnable = VK_FALSE;
//         colorBlending.logicOp = VK_LOGIC_OP_COPY;
//         colorBlending.attachmentCount = 1;
//         colorBlending.pAttachments = &colorBlendAttachment;
//         colorBlending.blendConstants[0] = 0.0f;
//         colorBlending.blendConstants[1] = 0.0f;
//         colorBlending.blendConstants[2] = 0.0f;
//         colorBlending.blendConstants[3] = 0.0f;

//         std::vector<VkDynamicState> dynamicStates = {
//             VK_DYNAMIC_STATE_VIEWPORT,
//             VK_DYNAMIC_STATE_SCISSOR
//         };
//         VkPipelineDynamicStateCreateInfo dynamicState{};
//         dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
//         dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
//         dynamicState.pDynamicStates = dynamicStates.data();

//         // pipline binding

//         VkDescriptorSetLayoutBinding binding = {};
//         binding.binding = 0;
//         binding.descriptorCount = 1;
//         binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
//         binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
//         binding.pImmutableSamplers = nullptr;

//         VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {};
//         layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
//         layoutCreateInfo.bindingCount = 1;
//         layoutCreateInfo.pBindings = &binding;
//         layoutCreateInfo.pNext = nullptr;

//         if (vkCreateDescriptorSetLayout(device, &layoutCreateInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
//         {
//             throw std::runtime_error("failed to create raytracing descriptor set layout!");
//         }

//         VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
//         pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
//         pipelineLayoutInfo.setLayoutCount = 1;
//         pipelineLayoutInfo.pushConstantRangeCount = 0;
//         pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

//         if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
//             throw std::runtime_error("failed to create pipeline layout!");
//         }

//         VkGraphicsPipelineCreateInfo pipelineInfo{};
//         pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
//         pipelineInfo.stageCount = 2;
//         pipelineInfo.pStages = shaderStages;
//         pipelineInfo.pVertexInputState = &vertexInputInfo;
//         pipelineInfo.pInputAssemblyState = &inputAssembly;
//         pipelineInfo.pViewportState = &viewportState;
//         pipelineInfo.pRasterizationState = &rasterizer;
//         pipelineInfo.pMultisampleState = &multisampling;
//         pipelineInfo.pColorBlendState = &colorBlending;
//         pipelineInfo.pDynamicState = &dynamicState;
//         pipelineInfo.layout = pipelineLayout;
//         pipelineInfo.renderPass = renderPass;
//         pipelineInfo.subpass = 0;
//         pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

//         if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
//             throw std::runtime_error("failed to create graphics pipeline!");
//         }

//         vkDestroyShaderModule(device, fragShaderModule, nullptr);
//         vkDestroyShaderModule(device, vertShaderModule, nullptr);

//          VkDescriptorPoolSize poolSize = {};
//         poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
//         poolSize.descriptorCount = 2;

//         VkDescriptorPoolCreateInfo poolCreateInfo = {};
//         poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
//         poolCreateInfo.poolSizeCount = 1;
//         poolCreateInfo.pPoolSizes = &poolSize;
//         poolCreateInfo.maxSets = 50;

//         if (vkCreateDescriptorPool(device, &poolCreateInfo, nullptr, &descriptorPool) != VK_SUCCESS)
//         {
//             throw std::runtime_error("failed to create raytracing descriptor pool!");
//         }

        




//         descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);

//         VkDescriptorSetLayout setLayouts[] = {descriptorSetLayout, descriptorSetLayout};

//         VkDescriptorSetAllocateInfo allocInfo = {};
//         allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
//         allocInfo.descriptorPool = descriptorPool;
//         allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
//         allocInfo.pSetLayouts = setLayouts;
        
//         if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()))
//         {
//             throw std::runtime_error("Failed to create raytracing descriptor set!");
//         }

//         imageSampler.resize(2);

//         for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
//         {

//             // image Sampler

//             VkSamplerCreateInfo samplerCreateInfo = {};
//             samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
//             samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
//             samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
//             samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
//             samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
//             samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
//             samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
//             samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_TRANSPARENT_BLACK;
//             samplerCreateInfo.anisotropyEnable = VK_FALSE;
//             samplerCreateInfo.maxAnisotropy = 1.0f;
//             samplerCreateInfo.compareEnable = VK_FALSE;
//             samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
//             samplerCreateInfo.minLod = 0.0f;
//             samplerCreateInfo.maxLod = VK_LOD_CLAMP_NONE;
//             samplerCreateInfo.mipLodBias = 0.0f;

//             if (vkCreateSampler(device, &samplerCreateInfo, nullptr, &imageSampler[i]) != VK_SUCCESS)
//             {
//                 throw std::runtime_error("failed to create sampler");
//             }
            

//             VkDescriptorImageInfo imageInfo = {};
//             imageInfo.imageView = raytracingStorageImageView[i];
//             imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
//             imageInfo.sampler = imageSampler[i];

//             VkWriteDescriptorSet writeDescriptorSet = {};
//             writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//             writeDescriptorSet.dstSet = descriptorSets[i];
//             writeDescriptorSet.dstBinding = 0;
//             writeDescriptorSet.dstArrayElement = 0;
//             writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
//             writeDescriptorSet.descriptorCount = 1;
//             writeDescriptorSet.pImageInfo = &imageInfo;

//             vkUpdateDescriptorSets(device, 1, &writeDescriptorSet, 0, nullptr);
//         }

//     }


//     void createChunkComputePipeline()
//     {
//         chunkComputeFence.resize(8);
//         for (int i = 0; i < 8; i++)
//         {
//             VkFenceCreateInfo fenceInfo = {};
//             fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
//             fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

//             if (vkCreateFence(device, &fenceInfo, nullptr, &chunkComputeFence[i]) != VK_SUCCESS)
//             {
//                 throw std::runtime_error("Failed to creaet compute semaphore");
//             }
//         }



//         VkDescriptorSetLayoutBinding layoutBinding{};
//         layoutBinding.binding = 0;
//         layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
//         layoutBinding.descriptorCount = 1;
//         layoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

//         VkDescriptorSetLayoutCreateInfo layoutInfo = {};
//         layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
//         layoutInfo.bindingCount = 1;
//         layoutInfo.pBindings = &layoutBinding;

//         if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &chunkComputeDescriptorSetLayout) != VK_SUCCESS)
//         {
//             throw std::runtime_error("Failed to create chunk compute descriptor set layout!");
//         }



//         VkDescriptorPoolSize poolSize{};
//         poolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
//         poolSize.descriptorCount = 8;

//         VkDescriptorPoolCreateInfo poolInfo = {};
//         poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
//         poolInfo.poolSizeCount = 1;
//         poolInfo.pPoolSizes = &poolSize;
//         poolInfo.maxSets = 50;

//         if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &chunkComputeDescriptorPool) != VK_SUCCESS)
//         {
//             throw std::runtime_error("Failed to create chunk compute descriptor set");
//         }


//         chunkComputeDescriptorSets.resize(8);

//         VkDescriptorSetLayout layouts[] = {chunkComputeDescriptorSetLayout,chunkComputeDescriptorSetLayout,chunkComputeDescriptorSetLayout,chunkComputeDescriptorSetLayout,chunkComputeDescriptorSetLayout,chunkComputeDescriptorSetLayout,chunkComputeDescriptorSetLayout,chunkComputeDescriptorSetLayout};
//         VkDescriptorSetAllocateInfo allocInfo{};
//         allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
//         allocInfo.descriptorPool = chunkComputeDescriptorPool;
//         allocInfo.descriptorSetCount = 8;
//         allocInfo.pSetLayouts = layouts;

//         if (vkAllocateDescriptorSets(device, &allocInfo, chunkComputeDescriptorSets.data()) != VK_SUCCESS)
//         {
//             throw std::runtime_error("failed to create chunk compute descriptor sets");
//         }

//         for (int i = 0; i < 8; i++)
//         {
//             VkDescriptorBufferInfo bufferInfo = {};
//             bufferInfo.buffer = chunkComputeBufferPool[i];
//             bufferInfo.offset = 0;
//             bufferInfo.range = 128*128*128;
            
//             VkWriteDescriptorSet descriptorWrite = {};
//             descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//             descriptorWrite.dstSet = chunkComputeDescriptorSets[i];
//             descriptorWrite.dstBinding = 0;
//             descriptorWrite.dstArrayElement = 0;
//             descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
//             descriptorWrite.descriptorCount = 1;
//             descriptorWrite.pBufferInfo = &bufferInfo;

//             vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);

//         }



//         VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
//         pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
//         pipelineLayoutCreateInfo.setLayoutCount = 1;
//         pipelineLayoutCreateInfo.pSetLayouts = &chunkComputeDescriptorSetLayout;

//         if (vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &chunkComputePipelineLayout) != VK_SUCCESS)
//         {
//             throw std::runtime_error("Failed to create chunk compute Pipeline layout!");
//         }

//         auto shaderCode = readFile("../comp.spv");

//         VkShaderModule shaderModule = createShaderModule(shaderCode);

//         VkPipelineShaderStageCreateInfo shaderStageInfo{};
//         shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
//         shaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
//         shaderStageInfo.module = shaderModule;
//         shaderStageInfo.pName = "main";

//         VkComputePipelineCreateInfo pipelineInfo{};
//         pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
//         pipelineInfo.stage = shaderStageInfo;
//         pipelineInfo.layout = chunkComputePipelineLayout;

//         if (vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &chunkComputePipeline) != VK_SUCCESS)
//         {
//             throw std::runtime_error("Failed to create chunk Compute pipeline");
//         }
//     }

//     void generateChunk(int i, glm::vec3 position)
//     {
//         vkWaitForFences(device, 1, &chunkComputeFence[i], VK_TRUE, UINT64_MAX);
//         VkCommandBufferAllocateInfo allocInfo = {};
//         allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
//         allocInfo.commandPool = commandPool;
//         allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
//         allocInfo.commandBufferCount = 1;
//         vkResetFences(device, 1, &chunkComputeFence[i]);
//         recordChunkComputeCommandBuffer(chunkComputeCommandBuffers[i], i);

//         VkSubmitInfo submitInfo = {};
//         submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
//         submitInfo.commandBufferCount = 1;
//         submitInfo.pCommandBuffers = &chunkComputeCommandBuffers[i];
//         // submitInfo.signalSemaphoreCount = 1;
//         // submitInfo.pSignalSemaphores = &chunkComputeSemaphores[i];

//         if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, chunkComputeFence[i]) != VK_SUCCESS)
//         {
//             throw std::runtime_error("failed to submit chunk compute!");
//         }
//     }

//     void recordChunkComputeCommandBuffer(VkCommandBuffer commandBuffer, int i)
//     {
//         VkCommandBufferBeginInfo beginInfo = {};
//         beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        
//         if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
//         {
//             throw std::runtime_error("Failed to begin chunk compute command buffer");
//         }

//         vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, chunkComputePipeline);

//         vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, chunkComputePipelineLayout, 0, 1, &chunkComputeDescriptorSets[i], 0, nullptr);

//         vkCmdDispatch(commandBuffer, 128, 128, 128);

//         if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
//         {
//             throw std::runtime_error("failed to end chunk compute command buffer");
//         }
//     }

//     void createChunkComputeBuffers()
//     {
//         chunkComputeBufferPool.resize(8);
//         chunkComputerBufferMemoryPool.resize(8);
//         chunkComputerBufferMappedMemoryPool.resize(8);

//         for (int i = 0; i < 8; i++)
//         {
//             VkBufferCreateInfo bufferInfo{};
//             bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
//             bufferInfo.size = 128*128*128;
//             bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
//             bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

//             if (vkCreateBuffer(device, &bufferInfo, nullptr, &chunkComputeBufferPool[i]) != VK_SUCCESS)
//             {
//                 throw std::runtime_error("Failed to create chunk compute shader buffer");
//             }

//             VkMemoryRequirements memRequirements;
//             vkGetBufferMemoryRequirements(device, chunkComputeBufferPool[i], &memRequirements);

//             VkMemoryAllocateInfo allocInfo = {};
//             allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
//             allocInfo.allocationSize = memRequirements.size;
//             allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, 
//                                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

//             VkPhysicalDeviceMemoryProperties memProperties;
//             vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

//             for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
//             {
//                 if ((memRequirements.memoryTypeBits & (1 << i)) &&
//                     (memProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) &&
//                     (memProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
//                     allocInfo.memoryTypeIndex = i;
//                     break;
//                 }
//             }

//             if (vkAllocateMemory(device, &allocInfo, nullptr, &chunkComputerBufferMemoryPool[i]) != VK_SUCCESS)
//             {
//                 throw std::runtime_error("Failed to allocate memory for chunk compute shader buffer!");
//             }

//             vkBindBufferMemory(device, chunkComputeBufferPool[i], chunkComputerBufferMemoryPool[i], 0);
//         }
//     }

//     void createFramebuffers() {
//         swapChainFramebuffers.resize(swapChainImageViews.size());

//         for (size_t i = 0; i < swapChainImageViews.size(); i++) {
//             VkImageView attachments[] = {
//                 swapChainImageViews[i]
//             };

//             VkFramebufferCreateInfo framebufferInfo{};
//             framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
//             framebufferInfo.renderPass = renderPass;
//             framebufferInfo.attachmentCount = 1;
//             framebufferInfo.pAttachments = attachments;
//             framebufferInfo.width = swapChainExtent.width;
//             framebufferInfo.height = swapChainExtent.height;
//             framebufferInfo.layers = 1;

//             if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
//                 throw std::runtime_error("failed to create framebuffer!");
//             }
//         }
//     }

//     void createCommandPool() {
//         QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

//         VkCommandPoolCreateInfo poolInfo{};
//         poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
//         poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
//         poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

//         if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
//             throw std::runtime_error("failed to create command pool!");
//         }
//     }

//     void createCommandBuffers() {
//         commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

//         VkCommandBufferAllocateInfo allocInfo{};
//         allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
//         allocInfo.commandPool = commandPool;
//         allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
//         allocInfo.commandBufferCount = (uint32_t) commandBuffers.size();

//         if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
//             throw std::runtime_error("failed to allocate command buffers!");
//         }


//         raytracingCommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

//         VkCommandBufferAllocateInfo raytracingAllocInfo{};
//         raytracingAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
//         raytracingAllocInfo.commandPool = commandPool;
//         raytracingAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
//         raytracingAllocInfo.commandBufferCount = (uint32_t) commandBuffers.size();

//         if (vkAllocateCommandBuffers(device, &raytracingAllocInfo, raytracingCommandBuffers.data()) != VK_SUCCESS) {
//             throw std::runtime_error("failed to allocate command buffers!");
//         }


//         chunkComputeCommandBuffers.resize(8);

//         VkCommandBufferAllocateInfo chunkComputeAllocInfo{};
//         chunkComputeAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
//         chunkComputeAllocInfo.commandPool = commandPool;
//         chunkComputeAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
//         chunkComputeAllocInfo.commandBufferCount = 8;

//         if (vkAllocateCommandBuffers(device, &chunkComputeAllocInfo, chunkComputeCommandBuffers.data()) != VK_SUCCESS) {
//             throw std::runtime_error("failed to allocate command buffers!");
//         }
//     }

//     void recordVoxelCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, uint8_t section)
//     {
//         VkCommandBufferBeginInfo beginInfo = {};
//         beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
//         beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
//         if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
//         {
//             throw std::runtime_error("failed to begin recording command buffer!");
//         }
 

//         vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, raytracingPipeline);

//         vkCmdBindDescriptorSets(
//             commandBuffer,
//             VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
//             raytracingPipelineLayout,
//             0,
//             1,
//             &raytracingDescriptorSets[currentFrame],
//             0,
//             nullptr
//         );

//         PFN_vkCmdTraceRaysKHR vkCmdTraceRaysKHR = reinterpret_cast<PFN_vkCmdTraceRaysKHR>(vkGetDeviceProcAddr(device, "vkCmdTraceRaysKHR"));
//         vkCmdTraceRaysKHR(
//             commandBuffer,
//             &raygenRegion,
//             &missRegion,
//             &hitRegion,
//             &callableRegion,
//             swapChainExtent.width,
//             swapChainExtent.height,
//             1
//         );
        

//         if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
//         {
//             throw std::runtime_error("failed to record raytracing command buffer!");
//         }
//     }

//     void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
//         VkCommandBufferBeginInfo beginInfo{};
//         beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

//         if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
//             throw std::runtime_error("failed to begin recording command buffer!");
//         }
//        if (imageIndex == 0)
//         {
//                 transitionImageLayout(commandBuffer, voxelChunkMapTexture, VK_FORMAT_R16_UINT, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1);
//                 transitionImageLayout(commandBuffer, voxelUpperTexture, VK_FORMAT_R8_UINT, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1);
//                 transitionImageLayout(commandBuffer, voxelTexture, VK_FORMAT_R8_UINT, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1);
//         UpdateVoxels(commandBuffer, voxelData, section);
//                 transitionImageLayout(commandBuffer, voxelTexture, VK_FORMAT_R8_UINT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL, 1);
//                 transitionImageLayout(commandBuffer, voxelUpperTexture, VK_FORMAT_R8_UINT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL, 1);
//                 transitionImageLayout(commandBuffer, voxelChunkMapTexture, VK_FORMAT_R16_UINT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL, 1);
//         }
//         VkRenderPassBeginInfo renderPassInfo{};
//         renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
//         renderPassInfo.renderPass = renderPass;
//         renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
//         renderPassInfo.renderArea.offset = {0, 0};
//         renderPassInfo.renderArea.extent = swapChainExtent;

//         VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
//         renderPassInfo.clearValueCount = 1;
//         renderPassInfo.pClearValues = &clearColor;

//         vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

//             vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

//             VkViewport viewport{};
//             viewport.x = 0.0f;
//             viewport.y = 0.0f;
//             viewport.width = (float) swapChainExtent.width;
//             viewport.height = (float) swapChainExtent.height;
//             viewport.minDepth = 0.0f;
//             viewport.maxDepth = 1.0f;
//             vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

//             VkRect2D scissor{};
//             scissor.offset = {0, 0};
//             scissor.extent = swapChainExtent;
//             vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

//             vkCmdBindDescriptorSets(
//                 commandBuffer,
//                 VK_PIPELINE_BIND_POINT_GRAPHICS,
//                 pipelineLayout,
//                 0,
//                 1,
//                 &descriptorSets[currentFrame],
//                 0,
//                 nullptr
//             );

//             vkCmdDraw(commandBuffer, 6, 1, 0, 0);

//         vkCmdEndRenderPass(commandBuffer);

//         if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
//             throw std::runtime_error("failed to record command buffer!");
//         }
//     }

//     void createSyncObjects() {
//         imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
//         renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
//         inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

//         VkSemaphoreCreateInfo semaphoreInfo{};
//         semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

//         VkFenceCreateInfo fenceInfo{};
//         fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
//         fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

//         for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
//             if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
//                 vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
//                 vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
//                 throw std::runtime_error("failed to create synchronization objects for a frame!");
//             }
//         }
//     }

//     void drawFrame() {
//         vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

//         uint32_t imageIndex;
//         VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

//         if (result == VK_ERROR_OUT_OF_DATE_KHR) {
//             recreateSwapChain();
//             return;
//         } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
//             throw std::runtime_error("failed to acquire swap chain image!");
//         }

//         vkResetFences(device, 1, &inFlightFences[currentFrame]);

//         vkResetCommandBuffer(commandBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
//         vkResetCommandBuffer(raytracingCommandBuffers[(currentFrame - 1 + MAX_FRAMES_IN_FLIGHT) % MAX_FRAMES_IN_FLIGHT], 0);

//         recordCommandBuffer(commandBuffers[currentFrame], imageIndex);
//         recordVoxelCommandBuffer(raytracingCommandBuffers[(currentFrame - 1 + MAX_FRAMES_IN_FLIGHT) % MAX_FRAMES_IN_FLIGHT], (currentFrame - 1 + MAX_FRAMES_IN_FLIGHT) % MAX_FRAMES_IN_FLIGHT, section);

//         VkSubmitInfo submitInfo{};
//         submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

//         VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
//         VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
//         submitInfo.waitSemaphoreCount = 1;
//         submitInfo.pWaitSemaphores = waitSemaphores;
//         submitInfo.pWaitDstStageMask = waitStages;


//         std::vector<VkCommandBuffer> commands;
//         commands.push_back(raytracingCommandBuffers[(currentFrame - 1 + MAX_FRAMES_IN_FLIGHT) % MAX_FRAMES_IN_FLIGHT]);
//         commands.push_back(commandBuffers[currentFrame]);

//         submitInfo.commandBufferCount = 2;
//         submitInfo.pCommandBuffers = commands.data();

//         VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
//         submitInfo.signalSemaphoreCount = 1;
//         submitInfo.pSignalSemaphores = signalSemaphores;

//         if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
//             throw std::runtime_error("failed to submit draw command buffer!");
//         }

        

//         VkPresentInfoKHR presentInfo{};
//         presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

//         presentInfo.waitSemaphoreCount = 1;
//         presentInfo.pWaitSemaphores = signalSemaphores;

//         VkSwapchainKHR swapChains[] = {swapChain};
//         presentInfo.swapchainCount = 1;
//         presentInfo.pSwapchains = swapChains;

//         presentInfo.pImageIndices = &imageIndex;

//         result = vkQueuePresentKHR(presentQueue, &presentInfo);

//         if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
//             framebufferResized = false;
//             recreateSwapChain();
//         } else if (result != VK_SUCCESS) {
//             throw std::runtime_error("failed to present swap chain image!");
//         }

//         currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
//         if (currentFrame == 0)
//         section = (section + 1) % 8;
//     }

//     VkCommandBuffer beginSingleTimeCommands() {
//         VkCommandBufferAllocateInfo allocInfo{};
//         allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
//         allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
//         allocInfo.commandPool = commandPool;
//         allocInfo.commandBufferCount = 1;

//         VkCommandBuffer commandBuffer;
//         vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

//         VkCommandBufferBeginInfo beginInfo{};
//         beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
//         beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

//         vkBeginCommandBuffer(commandBuffer, &beginInfo);

//         return commandBuffer;
//     }

//     void endSingleTimeCommands(VkCommandBuffer commandBuffer) {
//         vkEndCommandBuffer(commandBuffer);

//         VkSubmitInfo submitInfo{};
//         submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
//         submitInfo.commandBufferCount = 1;
//         submitInfo.pCommandBuffers = &commandBuffer;

//         vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
//         vkQueueWaitIdle(graphicsQueue);

//         vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
//     }

//     uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
//         VkPhysicalDeviceMemoryProperties memProperties;
//         vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

//         for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
//             if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
//                 return i;
//             }
//         }

//         throw std::runtime_error("failed to find suitable memory type!");
//     }

//     void CreateTransformUBO()
//     {
//         VkDeviceSize bufferSize = sizeof(TransformUBO);

//         VkBufferCreateInfo bufferInfo = {};
//         bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
//         bufferInfo.size = bufferSize;
//         bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
//         bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

//         if (vkCreateBuffer(device, &bufferInfo, nullptr, &uniformBuffer) != VK_SUCCESS)
//         {
//             throw std::runtime_error("failed to create uniform buffer");
//         }

//         VkMemoryRequirements memRequirements;
//         vkGetBufferMemoryRequirements(device, uniformBuffer, &memRequirements);

//         VkMemoryAllocateInfo allocInfo{};
//         allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
//         allocInfo.allocationSize = memRequirements.size;
//         allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

//         if (vkAllocateMemory(device, &allocInfo, nullptr, &uniformBufferMemory) != VK_SUCCESS)
//         {
//             throw std::runtime_error("Failed to allocate uniform buffer memory!");
//         }

//         vkBindBufferMemory(device, uniformBuffer, uniformBufferMemory, 0);

  
//         vkMapMemory(device, uniformBufferMemory, 0, sizeof(TransformUBO), 0, &uniformBuffersMapped);
//         memcpy(uniformBuffersMapped, &ubo, sizeof(TransformUBO));

//         ubo.view = glm::mat4(1.0);
//         ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 1000.0f);
//         ubo.proj[1][1] *= -1;
//         ubo.proj = glm::inverse(ubo.proj);
        
//     }

//     void onMouseMove(double xPos, double yPos) {
//         mouseX = (float)xPos;
//         mouseY = (float)yPos;
//     }

//     // Example of key press handler
//     bool isKeyPressed(int key) {
//         return glfwGetKey(window, key) == GLFW_PRESS;
//     }

//     // Example of mouse button press handler
//     bool isMouseButtonPressed(int button) {
//         return glfwGetMouseButton(window, button) == GLFW_PRESS;
//     }

//     // Function to hide the cursor
//     void hideCursor() {
//         glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
//     }

//     // Function to show the cursor
//     void showCursor() {
//         glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
//     }

//     bool firstMouse = true;

//     void UpdateUBO()
//     {
//         printf("POSITION: %f %f %f\n", cameraPosition.x, cameraPosition.y, cameraPosition.z);
//         // printf("DELTA TIME: %f\n", deltaTime);

//         // cameraPosition.z += cameraVelocity.z * deltaTime;
//         // cameraTargetPoint.z += cameraVelocity.z * deltaTime;

//         glm::ivec3 intCameraPosition = glm::ivec3(cameraPosition) * -1;


//         uint16_t materials = 0;
//         // for (int x = -1; x < 2; x++)
//         // {
//         //     for (int y = -1; y < 2; y++)
//         //     {
//         //         materials += (voxelData[(intCameraPosition.z+20) * 1024*1024 + (intCameraPosition.y+y)*1024 + (intCameraPosition.x+x)] & 0b11111);
//         //     }
//         // }



//         // printf("VOXEL UNDER %i\n", voxelData[(intCameraPosition.z) * 1024*1024 + intCameraPosition.y*1024 + intCameraPosition.x] & 0b11111);
//         if (materials > 0)
//         {
//             cameraVelocity.z = 0;
//         }
//         else
//         {

//             cameraVelocity.z -= 9.8 * 7 * deltaTime;
//         }

//         if (isMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT)) {
//         // Hide the cursor and capture mouse movement
//         hideCursor();

//         float deltaTime = 0.1f;

//         cameraTargetPoint.z = cameraPosition.z;
//         if (isKeyPressed(GLFW_KEY_S)) { cameraPosition += movementSpeed * deltaTime * glm::normalize(cameraTargetPoint - cameraPosition); }
//             if (isKeyPressed(GLFW_KEY_W)) { cameraPosition -= movementSpeed * deltaTime * glm::normalize(cameraTargetPoint - cameraPosition); }
//             if (isKeyPressed(GLFW_KEY_D)) { cameraPosition -= glm::normalize(glm::cross(cameraTargetPoint - cameraPosition, glm::vec3(0.0f, 0.0f, 1.0f))) * movementSpeed * deltaTime; }
//             if (isKeyPressed(GLFW_KEY_A)) { cameraPosition += glm::normalize(glm::cross(cameraTargetPoint - cameraPosition, glm::vec3(0.0f, 0.0f, 1.0f))) * movementSpeed * deltaTime; }

//             // Q and E for vertical movement
//             if (isKeyPressed(GLFW_KEY_Q)) { cameraPosition.z -= movementSpeed * 2 * deltaTime; }
//             if (isKeyPressed(GLFW_KEY_SPACE)) { cameraPosition.z += movementSpeed * 2 * deltaTime; }

//             double currentMouseX, currentMouseY;
//             glfwGetCursorPos(window, &currentMouseX, &currentMouseY);

//             if (firstMouse) {
//                 lastMouseX = static_cast<float>(currentMouseX);
//                 lastMouseY = static_cast<float>(currentMouseY);
//                 firstMouse = false;
//             }

//             // Calculate the offset
//             float offsetX = static_cast<float>(currentMouseX - lastMouseX);
//             float offsetY = static_cast<float>(lastMouseY - currentMouseY);  // Reverse Y for correct orientation
//             lastMouseX = static_cast<float>(currentMouseX);
//             lastMouseY = static_cast<float>(currentMouseY);

//             // Apply sensitivity
//             offsetX *= mouseSensitivity;
//             offsetY *= mouseSensitivity;

//             yaw -= offsetX;
//             pitch -= offsetY;

//             // Constrain pitch to prevent flipping
//             if (pitch > 89.0f) pitch = 89.0f;
//             if (pitch < -89.0f) pitch = -89.0f;

//             // Update the camera target based on yaw and pitch
//             glm::vec3 direction;
//             direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
//             direction.z = sin(glm::radians(pitch));
//             direction.y = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
//             direction = glm::normalize(direction);

//             cameraTargetPoint = cameraPosition + direction;
//         } else {
//             // Release the cursor and reset first mouse flag
//             showCursor();
//             firstMouse = true;
//         }

//         ubo.view = glm::inverse(glm::lookAt(cameraPosition, cameraTargetPoint, glm::vec3(0.0f, 0.0f, 1.0f)));

//         memcpy(uniformBuffersMapped, &ubo, sizeof(TransformUBO));
//     }
    

//     void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels) {
//         VkCommandBuffer commandBuffer = beginSingleTimeCommands();

//         VkImageMemoryBarrier barrier{};
//         barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
//         barrier.oldLayout = oldLayout;
//         barrier.newLayout = newLayout;
//         barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//         barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//         barrier.image = image;
//         barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//         barrier.subresourceRange.baseMipLevel = 0;
//         barrier.subresourceRange.levelCount = mipLevels;
//         barrier.subresourceRange.baseArrayLayer = 0;
//         barrier.subresourceRange.layerCount = 1;

//         VkPipelineStageFlags sourceStage;
//         VkPipelineStageFlags destinationStage;

//         if (oldLayout == VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
//             barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT; // The image is being read in shaders
//             barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT; // The image will be written to during transfer

//             sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; // Wait for fragment shader (or compute shader) to finish
//             destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT; // Transfer operation will be the next stage
//         } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL) {
//             barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT; // The image was written to during the transfer operation
//             barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT; // The image will be read by shaders (for sampling)

//             sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT; // Wait for the transfer operation to finish
//             destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; // Fragment shader (or compute shader) will use the image
//         } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL) {
//             barrier.srcAccessMask = 0; // No access is required from the old layout
//             barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT; // The image will be read by shaders (for sampling)

//             sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT; // No need to wait for previous stages since it's undefined
//             destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; // The image will be used in a fragment shader
//         } else if (oldLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR && newLayout == VK_IMAGE_LAYOUT_GENERAL) {
//             barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
//             barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;

//             sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//             destinationStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
//         } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_GENERAL) {
//             barrier.srcAccessMask = 0;
//             barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;

//             sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
//             destinationStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
//         } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
//             barrier.srcAccessMask = 0;
//             barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

//             sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
//             destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//         } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
//             barrier.srcAccessMask = 0;
//             barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

//             sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
//             destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
//         } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
//             barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
//             barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

//             sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
//             destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
//         } else {
//             throw std::invalid_argument("unsupported layout transition!");
//         }

//         vkCmdPipelineBarrier(
//             commandBuffer,
//             sourceStage, destinationStage,
//             0,
//             0, nullptr,
//             0, nullptr,
//             1, &barrier
//         );

//         endSingleTimeCommands(commandBuffer);
//     }

//     void transitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels) {

//         VkImageMemoryBarrier barrier{};
//         barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
//         barrier.oldLayout = oldLayout;
//         barrier.newLayout = newLayout;
//         barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//         barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//         barrier.image = image;
//         barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//         barrier.subresourceRange.baseMipLevel = 0;
//         barrier.subresourceRange.levelCount = mipLevels;
//         barrier.subresourceRange.baseArrayLayer = 0;
//         barrier.subresourceRange.layerCount = 1;

//         VkPipelineStageFlags sourceStage;
//         VkPipelineStageFlags destinationStage;
//         if (oldLayout == VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
//             barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT; // The image is being read in shaders
//             barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT; // The image will be written to during transfer

//             sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; // Wait for fragment shader (or compute shader) to finish
//             destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT; // Transfer operation will be the next stage
//         } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL) {
//             barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT; // The image was written to during the transfer operation
//             barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT; // The image will be read by shaders (for sampling)

//             sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT; // Wait for the transfer operation to finish
//             destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; // Fragment shader (or compute shader) will use the image
//         } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL) {
//             barrier.srcAccessMask = 0; // No access is required from the old layout
//             barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT; // The image will be read by shaders (for sampling)

//             sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT; // No need to wait for previous stages since it's undefined
//             destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; // The image will be used in a fragment shader
//         } else if (oldLayout == VK_IMAGE_LAYOUT_GENERAL && newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
//             barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
//             barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;

//             sourceStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
//             destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
//         } else if (oldLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR && newLayout == VK_IMAGE_LAYOUT_GENERAL) {
//             barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
//             barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;

//             sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//             destinationStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
//         } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_GENERAL) {
//             barrier.srcAccessMask = 0;
//             barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;

//             sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
//             destinationStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
//         } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
//             barrier.srcAccessMask = 0;
//             barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

//             sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
//             destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//         } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
//             barrier.srcAccessMask = 0;
//             barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

//             sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
//             destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
//         } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
//             barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
//             barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

//             sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
//             destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
//         } else {
//             throw std::invalid_argument("unsupported layout transition!");
//         }

//         vkCmdPipelineBarrier(
//             commandBuffer,
//             sourceStage, destinationStage,
//             0,
//             0, nullptr,
//             0, nullptr,
//             1, &barrier
//         );
//     }

//     VkShaderModule createShaderModule(const std::vector<char>& code) {
//         VkShaderModuleCreateInfo createInfo{};
//         createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
//         createInfo.codeSize = code.size();
//         createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

//         VkShaderModule shaderModule;
//         if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
//             throw std::runtime_error("failed to create shader module!");
//         }

//         return shaderModule;
//     }

//     VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
//         for (const auto& availableFormat : availableFormats) {
//             if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
//                 return availableFormat;
//             }
//         }

//         return availableFormats[0];
//     }

//     VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
//         for (const auto& availablePresentMode : availablePresentModes) {
//             if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
//                 return availablePresentMode;
//             }
//         }

//         return VK_PRESENT_MODE_FIFO_KHR;
//     }

//     VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
//         if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
//             return capabilities.currentExtent;
//         } else {
//             int width, height;
//             glfwGetFramebufferSize(window, &width, &height);

//             VkExtent2D actualExtent = {
//                 static_cast<uint32_t>(width),
//                 static_cast<uint32_t>(height)
//             };

//             actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
//             actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

//             return actualExtent;
//         }
//     }

//     SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
//         SwapChainSupportDetails details;

//         vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

//         uint32_t formatCount;
//         vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

//         if (formatCount != 0) {
//             details.formats.resize(formatCount);
//             vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
//         }

//         uint32_t presentModeCount;
//         vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

//         if (presentModeCount != 0) {
//             details.presentModes.resize(presentModeCount);
//             vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
//         }

//         return details;
//     }

//     bool isDeviceSuitable(VkPhysicalDevice device) {
//         QueueFamilyIndices indices = findQueueFamilies(device);

//         bool extensionsSupported = checkDeviceExtensionSupport(device);

//         bool swapChainAdequate = false;
//         if (extensionsSupported) {
//             SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
//             swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
//         }

//         return indices.isComplete() && extensionsSupported && swapChainAdequate;
//     }

//     bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
//         uint32_t extensionCount;
//         vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

//         std::vector<VkExtensionProperties> availableExtensions(extensionCount);
//         vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

//         std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

//         for (const auto& extension : availableExtensions) {
//             requiredExtensions.erase(extension.extensionName);
//         }

//         return requiredExtensions.empty();
//     }

//     QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
//         QueueFamilyIndices indices;

//         uint32_t queueFamilyCount = 0;
//         vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

//         std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
//         vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

//         int i = 0;
//         for (const auto& queueFamily : queueFamilies) {
//             if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
//                 indices.graphicsFamily = i;
//             }

//             VkBool32 presentSupport = false;
//             vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

//             if (presentSupport) {
//                 indices.presentFamily = i;
//             }

//             if (indices.isComplete()) {
//                 break;
//             }

//             i++;
//         }

//         return indices;
//     }

//     std::vector<const char*> getRequiredExtensions() {
//         uint32_t glfwExtensionCount = 0;
//         const char** glfwExtensions;
//         glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

//         std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

//         if (enableValidationLayers) {
//             extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
//         }

//         return extensions;
//     }

//     bool checkValidationLayerSupport() {
//         uint32_t layerCount;
//         vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

//         std::vector<VkLayerProperties> availableLayers(layerCount);
//         vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

//         for (const char* layerName : validationLayers) {
//             bool layerFound = false;

//             for (const auto& layerProperties : availableLayers) {
//                 if (strcmp(layerName, layerProperties.layerName) == 0) {
//                     layerFound = true;
//                     break;
//                 }
//             }

//             if (!layerFound) {
//                 return false;
//             }
//         }

//         return true;
//     }

//     static std::vector<char> readFile(const std::string& filename) {
//         std::ifstream file(filename, std::ios::ate | std::ios::binary);

//         if (!file.is_open()) {
//             std::cout << filename << std::endl;
//             throw std::runtime_error("failed to open file!");
//         }

//         size_t fileSize = (size_t) file.tellg();
//         std::vector<char> buffer(fileSize);

//         file.seekg(0);
//         file.read(buffer.data(), fileSize);

//         file.close();

//         return buffer;
//     }

//     static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
//         std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

//         return VK_FALSE;
//     }
// };

// int main() {
//     VoxelEngine app;
    
//     try {
//         app.run();
//     } catch (const std::exception& e) {
//         std::cerr << e.what() << std::endl;
//         return EXIT_FAILURE;
//     }

//     return EXIT_SUCCESS;
// }



#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

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

#define FNL_IMPL
#include "FastNoiseLite.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONCe
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

static float semiRandomFloat(float x, float y, float z);

const uint32_t WIDTH = 1920;
const uint32_t HEIGHT = 1280;

struct TransformUBO
{
    glm::mat4 view;
    glm::mat4 proj;
};

const int MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
    VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME
};

#ifdef NDEBUG
const bool enableValidationLayers = true;
#else
const bool enableValidationLayers = true;
#endif

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

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
// ================

#define voxel_mat(voxel) (voxel & 0b11111)
#define MAT_IS_STONE(mat) (mat >= MAT_STONE && mat <= MAT_STONE3)
#define MAT_HAS_COLLISION(mat) (mat > MAT_AIR && mat < MAT_FLOWER)

class VoxelEngine {
public:
    void run() {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    GLFWwindow* window;

    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkSurfaceKHR surface;

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;

    VkQueue graphicsQueue;
    VkQueue presentQueue;

    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;

    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkCommandBuffer> raytracingCommandBuffers;

    std::vector<VkCommandBuffer> commandBufferPairs;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
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

    std::vector<VkImage> raytracingStorageImage;
    std::vector<VkImageView> raytracingStorageImageView;

    std::vector<VkSampler> imageSampler;

    VkBuffer uniformBuffer;
    VkDeviceMemory uniformBufferMemory;
    TransformUBO ubo{};

    void* uniformBuffersMapped;

    // Camera

    glm::vec3 cameraTargetPoint = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 cameraPosition = glm::vec3(-512, -512, -512);//glm::vec3(-512.0f, -512.0f, -512.0f);

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

    VkImage voxelTexture;
    VkDeviceMemory voxelTexturesMemory;
    VkImageView voxelImageView;
    VkSampler voxelTextureSampler;

    VkBuffer voxelStagingBuffer;
    VkDeviceMemory voxelStagingBufferMemory;


    VkImage voxelUpperTexture;
    VkDeviceMemory voxelUpperTexturesMemory;
    VkImageView voxelUpperImageView;

    VkBuffer voxelUpperStagingBuffer;
    VkDeviceMemory voxelUpperStagingBufferMemory;



    VkImage voxelChunkMapTexture;
    VkDeviceMemory voxelChunkMapTexturesMemory;
    VkImageView voxelChunkMapImageView;

    VkBuffer voxelChunkMapStagingBuffer;
    VkDeviceMemory voxelChunkMapStagingBufferMemory;

    uint8_t* voxelData;
    uint8_t* voxelUpperData;
    uint16_t* voxelChunkMapData;

    FastNoiseLite noise;

    double deltaTime = 0;
    double lastTime = 0;

    void initWindow() {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        window = glfwCreateWindow(WIDTH, HEIGHT, "Voxels", nullptr, nullptr);
        // const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        // glfwSetWindowPos(window, (mode->width - WIDTH) / 2, 32);

        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    }

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto app = reinterpret_cast<VoxelEngine*>(glfwGetWindowUserPointer(window));
        app->framebufferResized = true;
    }

    void initVulkan() {
        createInstance();
        setupDebugMessenger();
        createSurface();
        pickPhysicalDevice();
        createLogicalDevice();
        createSwapChain();
        createTransformUBO();
        createImageViews();
        createRenderPass();
        createCommandPool();
        createCommandBuffers();
        createRaytracingStorageImage();
        createGraphicsPipeline();
        createFramebuffers();
        createSyncObjects();

        createVoxelResources();
        createRaytracingPipeline();
        createRaytracingRegions();
    }

    void mainLoop() {
        while (!glfwWindowShouldClose(window)) {
            deltaTime = glfwGetTime() - lastTime;
            lastTime = glfwGetTime();
            glfwPollEvents();
            
            drawFrame();
        }

        vkDeviceWaitIdle(device);
    }

    void cleanupSwapChain() {
        for (auto framebuffer : swapChainFramebuffers) {
            vkDestroyFramebuffer(device, framebuffer, nullptr);
        }

        for (auto imageView : swapChainImageViews) {
            vkDestroyImageView(device, imageView, nullptr);
        }

        vkDestroySwapchainKHR(device, swapChain, nullptr);
    }

    void cleanup() {
        cleanupSwapChain();

        vkDestroyPipeline(device, graphicsPipeline, nullptr);
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);

        vkDestroyRenderPass(device, renderPass, nullptr);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(device, inFlightFences[i], nullptr);
        }

        vkDestroyCommandPool(device, commandPool, nullptr);

        vkDestroyDevice(device, nullptr);

        if (enableValidationLayers) {
            DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        }

        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);

        glfwDestroyWindow(window);

        glfwTerminate();
    }

    void recreateSwapChain() {
        int width = 0, height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(window, &width, &height);
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(device);

        cleanupSwapChain();

        createSwapChain();
        createImageViews();
        createFramebuffers();
        createRaytracingStorageImage();
    }

    void createInstance() {
        if (enableValidationLayers && !checkValidationLayerSupport()) {
            throw std::runtime_error("validation layers requested, but not available!");
        }

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Voxels";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_3;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        auto extensions = getRequiredExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();

            populateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
        } else {
            createInfo.enabledLayerCount = 0;

            createInfo.pNext = nullptr;
        }

        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
            throw std::runtime_error("failed to create instance!");
        }
    }

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = /*VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |*/ VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
    }

    void setupDebugMessenger() {
        if (!enableValidationLayers) return;

        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        populateDebugMessengerCreateInfo(createInfo);

        if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug messenger!");
        }
    }

    void createSurface() {
        if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
    }

    void pickPhysicalDevice() {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

        if (deviceCount == 0) {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        for (const auto& device : devices) {
            if (isDeviceSuitable(device)) {
                physicalDevice = device;
                break;
            }
        }

        if (physicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error("failed to find a suitable GPU!");
        }
    }

    void createLogicalDevice() {
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }


        // Enabling Raytracing Features

        VkPhysicalDeviceBufferAddressFeaturesEXT bufferDeviceFeatures = {};
        bufferDeviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_ADDRESS_FEATURES_EXT;
        bufferDeviceFeatures.bufferDeviceAddress = VK_TRUE;
        bufferDeviceFeatures.pNext = nullptr;

        VkPhysicalDeviceDescriptorIndexingFeaturesEXT descriptorIndexingFeatures = {};
        descriptorIndexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT;
        descriptorIndexingFeatures.descriptorBindingPartiallyBound = VK_TRUE;
        descriptorIndexingFeatures.descriptorBindingVariableDescriptorCount = VK_TRUE;
        descriptorIndexingFeatures.pNext = &bufferDeviceFeatures;

        VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingFeatures = {};
        rayTracingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
        rayTracingFeatures.rayTracingPipeline = VK_TRUE;
        rayTracingFeatures.pNext = &descriptorIndexingFeatures;

        VkPhysicalDeviceAccelerationStructureFeaturesKHR accelStructFeatures = {};
        accelStructFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
        accelStructFeatures.accelerationStructure = VK_TRUE;
        accelStructFeatures.pNext = &rayTracingFeatures;

        VkPhysicalDeviceFeatures2 deviceFeatures2 = {};
        deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        deviceFeatures2.pNext = &accelStructFeatures;

        VkPhysicalDeviceProperties2 deviceProperties2 = {};
        deviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        deviceProperties2.pNext = &accelStructFeatures;
        deviceFeatures2.features.tessellationShader = VK_TRUE;
        deviceFeatures2.features.geometryShader = VK_TRUE;

        vkGetPhysicalDeviceProperties2(physicalDevice, &deviceProperties2);

        if (!rayTracingFeatures.rayTracingPipeline)
        {
            std::cerr << "Ray tracing pipeline is not supported on this device" << std::endl;
            exit(1);
        }

        VkPhysicalDeviceFeatures deviceFeatures{};

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();

        createInfo.pEnabledFeatures = &deviceFeatures;

        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();
        createInfo.pNext = &deviceFeatures2;

        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        } else {
            createInfo.enabledLayerCount = 0;
        }

        if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
            throw std::runtime_error("failed to create logical device!");
        }

        vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
        vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
    }

    void createSwapChain() {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;

        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
        uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

        if (indices.graphicsFamily != indices.presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;

        if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
            throw std::runtime_error("failed to create swap chain!");
        }

        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
        swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

        swapChainImageFormat = surfaceFormat.format;
        swapChainExtent = extent;
    }

    void createImageViews() {
        swapChainImageViews.resize(swapChainImages.size());

        for (size_t i = 0; i < swapChainImages.size(); i++) {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = swapChainImages[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = swapChainImageFormat;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create image views!");
            }
        }
    }

    void createRenderPass() {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = swapChainImageFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass!");
        }
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
        vkGetPhysicalDeviceProperties2(physicalDevice, &deviceProperties2);

        VkDeviceSize handleSize = raytracingPipelineProperties.shaderGroupHandleSize;
        VkDeviceSize handleSizeAligned = ALIGN_UP(handleSize, raytracingPipelineProperties.shaderGroupBaseAlignment);
        sbtSize = handleSizeAligned * 2;

        createBuffer(device, physicalDevice, sbtSize, VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, sbtBuffer, sbtMemory);


        std::vector<uint8_t> shaderHandleStorage(sbtSize);

        PFN_vkGetRayTracingShaderGroupHandlesKHR vkGetRayTracingShaderGroupHandlesKHR = reinterpret_cast<PFN_vkGetRayTracingShaderGroupHandlesKHR>(vkGetDeviceProcAddr(device, "vkGetRayTracingShaderGroupHandlesKHR"));
        vkGetRayTracingShaderGroupHandlesKHR(device, raytracingPipeline, 0, 2, sbtSize, shaderHandleStorage.data());

        void* mappedData;
        vkMapMemory(device, sbtMemory, 0, sbtSize, 0, &mappedData);
        memcpy(mappedData, shaderHandleStorage.data(), sbtSize);
        vkUnmapMemory(device, sbtMemory);


        VkBufferDeviceAddressInfo bufferAddressInfo = {};
        bufferAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
        bufferAddressInfo.buffer = sbtBuffer;
        VkDeviceAddress sbtAddress = vkGetBufferDeviceAddress(device, &bufferAddressInfo);

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
        
        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            VkImageCreateInfo imageCreateInfo = {};
            imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
            imageCreateInfo.format = VK_FORMAT_R16G16B16A16_UNORM;
            imageCreateInfo.extent.width = swapChainExtent.width;
            imageCreateInfo.extent.height = swapChainExtent.height;
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

            if (vkCreateImage(device, &imageCreateInfo, nullptr, &raytracingStorageImage[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create raytracing storage image!");
            }

            VkMemoryRequirements memRequirements;
            vkGetImageMemoryRequirements(device, raytracingStorageImage[i], &memRequirements);

            VkMemoryAllocateInfo allocInfo = {};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = memRequirements.size;

            VkPhysicalDeviceMemoryProperties memProperties;
            vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

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
            if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to allocate memory for image!");
            }

            vkBindImageMemory(device, raytracingStorageImage[i], imageMemory, 0);

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

            if (vkCreateImageView(device, &viewCreateInfo, nullptr, &raytracingStorageImageView[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to creaet raytracing image view!");
            }

            transitionImageLayout(raytracingStorageImage[i], VK_FORMAT_R16G16B16A16_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, 1);
        }
        
    }

    void createVoxelResources()
    {
        // Create the image (texture)
        VkImageCreateInfo imageInfo = {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_3D;
        imageInfo.extent.width = 1024;
        imageInfo.extent.height = 1024;
        imageInfo.extent.depth = 1024;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = VK_FORMAT_R8_UINT;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

        if (vkCreateImage(device, &imageInfo, nullptr, &voxelTexture) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture image!");
        }

        // Allocate memory for the image
        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(device, voxelTexture, &memRequirements);

        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        if (vkAllocateMemory(device, &allocInfo, nullptr, &voxelTexturesMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate memory for texture image!");
        }

        vkBindImageMemory(device, voxelTexture, voxelTexturesMemory, 0);

        VkImageViewCreateInfo viewInfo = {};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = voxelTexture;
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

        if (vkCreateImageView(device, &viewInfo, nullptr, &voxelImageView) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture image view!");
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

        vkCreateSampler(device, &samplerCreateInfo, nullptr, &voxelTextureSampler);


        // Staging buffer to update the texture

        VkBufferCreateInfo bufferCreateInfo = {};
        bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferCreateInfo.size = 1024*1024*1024;
        bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(device, &bufferCreateInfo, nullptr, &voxelStagingBuffer) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create voxel staging buffer");
        }

        VkMemoryRequirements stagingMemRequirements;
        vkGetBufferMemoryRequirements(device, voxelStagingBuffer, &stagingMemRequirements);

        VkMemoryAllocateInfo stagingAllocInfo = {};
        stagingAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        stagingAllocInfo.allocationSize = stagingMemRequirements.size;
        stagingAllocInfo.memoryTypeIndex = findMemoryType(physicalDevice, stagingMemRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        vkAllocateMemory(device, &stagingAllocInfo, nullptr, &voxelStagingBufferMemory);
        vkBindBufferMemory(device, voxelStagingBuffer, voxelStagingBufferMemory, 0);
        
        transitionImageLayout(voxelTexture, VK_FORMAT_R8_UINT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL, 1);

        GenerateTerrain();

        // Upper
        {


        // Create the image (texture)
        VkImageCreateInfo upperImageInfo = {};
        upperImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        upperImageInfo.imageType = VK_IMAGE_TYPE_3D;
        upperImageInfo.extent.width = 64;
        upperImageInfo.extent.height = 64;
        upperImageInfo.extent.depth = 64;
        upperImageInfo.mipLevels = 1;
        upperImageInfo.arrayLayers = 1;
        upperImageInfo.format = VK_FORMAT_R8_UINT;
        upperImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        upperImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        upperImageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        upperImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        upperImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

        if (vkCreateImage(device, &upperImageInfo, nullptr, &voxelUpperTexture) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture image!");
        }

        // Allocate memory for the image
        VkMemoryRequirements upperMemRequirements;
        vkGetImageMemoryRequirements(device, voxelUpperTexture, &upperMemRequirements);

        VkMemoryAllocateInfo upperAllocInfo = {};
        upperAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        upperAllocInfo.allocationSize = upperMemRequirements.size;
        upperAllocInfo.memoryTypeIndex = findMemoryType(physicalDevice, upperMemRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        if (vkAllocateMemory(device, &upperAllocInfo, nullptr, &voxelUpperTexturesMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate memory for texture image!");
        }

        vkBindImageMemory(device, voxelUpperTexture, voxelUpperTexturesMemory, 0);

        VkImageViewCreateInfo upperViewInfo = {};
        upperViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        upperViewInfo.image = voxelUpperTexture;
        upperViewInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
        upperViewInfo.format = VK_FORMAT_R8_UINT;
        upperViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        upperViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        upperViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        upperViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        upperViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        upperViewInfo.subresourceRange.baseMipLevel = 0;
        upperViewInfo.subresourceRange.levelCount = 1;
        upperViewInfo.subresourceRange.baseArrayLayer = 0;
        upperViewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device, &upperViewInfo, nullptr, &voxelUpperImageView) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture image view!");
        }


        // Staging buffer to update the texture

        VkBufferCreateInfo upperBufferCreateInfo = {};
        upperBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        upperBufferCreateInfo.size = 64*64*64;
        upperBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        upperBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(device, &upperBufferCreateInfo, nullptr, &voxelUpperStagingBuffer) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create voxel staging buffer");
        }

        VkMemoryRequirements upperStagingMemRequirements;
        vkGetBufferMemoryRequirements(device, voxelUpperStagingBuffer, &upperStagingMemRequirements);

        VkMemoryAllocateInfo upperStagingAllocInfo = {};
        upperStagingAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        upperStagingAllocInfo.allocationSize = upperStagingMemRequirements.size;
        upperStagingAllocInfo.memoryTypeIndex = findMemoryType(physicalDevice, upperStagingMemRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        vkAllocateMemory(device, &upperStagingAllocInfo, nullptr, &voxelUpperStagingBufferMemory);
        vkBindBufferMemory(device, voxelUpperStagingBuffer, voxelUpperStagingBufferMemory, 0);
        
        transitionImageLayout(voxelUpperTexture, VK_FORMAT_R8_UINT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL, 1);

        // Voxel Data

        voxelUpperData = new uint8_t[64*64*64]();

    }



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

        if (vkCreateImage(device, &voxelChunkMapImageInfo, nullptr, &voxelChunkMapTexture) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture image!");
        }

        // Allocate memory for the image
        VkMemoryRequirements voxelChunkMapMemRequirements;
        vkGetImageMemoryRequirements(device, voxelChunkMapTexture, &voxelChunkMapMemRequirements);

        VkMemoryAllocateInfo voxelChunkMapAllocInfo = {};
        voxelChunkMapAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        voxelChunkMapAllocInfo.allocationSize = voxelChunkMapMemRequirements.size;
        voxelChunkMapAllocInfo.memoryTypeIndex = findMemoryType(physicalDevice, voxelChunkMapMemRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        if (vkAllocateMemory(device, &voxelChunkMapAllocInfo, nullptr, &voxelChunkMapTexturesMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate memory for texture image!");
        }

        vkBindImageMemory(device, voxelChunkMapTexture, voxelChunkMapTexturesMemory, 0);

        VkImageViewCreateInfo voxelChunkMapViewInfo = {};
        voxelChunkMapViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        voxelChunkMapViewInfo.image = voxelChunkMapTexture;
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

        if (vkCreateImageView(device, &voxelChunkMapViewInfo, nullptr, &voxelChunkMapImageView) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture image view!");
        }


        // Staging buffer to update the texture

        VkBufferCreateInfo voxelChunkMapBufferCreateInfo = {};
        voxelChunkMapBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        voxelChunkMapBufferCreateInfo.size = 8*8*8*2;
        voxelChunkMapBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        voxelChunkMapBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(device, &voxelChunkMapBufferCreateInfo, nullptr, &voxelChunkMapStagingBuffer) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create voxel staging buffer");
        }

        VkMemoryRequirements voxelChunkMapStagingMemRequirements;
        vkGetBufferMemoryRequirements(device, voxelChunkMapStagingBuffer, &voxelChunkMapStagingMemRequirements);

        VkMemoryAllocateInfo voxelChunkMapStagingAllocInfo = {};
        voxelChunkMapStagingAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        voxelChunkMapStagingAllocInfo.allocationSize = voxelChunkMapStagingMemRequirements.size;
        voxelChunkMapStagingAllocInfo.memoryTypeIndex = findMemoryType(physicalDevice, voxelChunkMapStagingMemRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        vkAllocateMemory(device, &voxelChunkMapStagingAllocInfo, nullptr, &voxelChunkMapStagingBufferMemory);
        vkBindBufferMemory(device, voxelChunkMapStagingBuffer, voxelChunkMapStagingBufferMemory, 0);
        
        transitionImageLayout(voxelChunkMapTexture, VK_FORMAT_R16_UINT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL, 1);

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
        // Voxel Data
        auto it = std::chrono::system_clock::now();
        voxelData = new uint8_t[1024 * 1024 * 1024]();
#define vox(x,y,z) voxelData[(z) * 1024 * 1024 + (y) * 1024 + (x)]

#define TERRAIN_SCALE 2
#define TERRAIN_AREA (1024)

        //printf("hwc: %i", std::thread::hardware_concurrency());
        printf("Generating\n");
        const int WORKERS = 20;
        const int SLICE_THICKNESS = TERRAIN_AREA / WORKERS;

        std::thread* threads[WORKERS]{};

        for (int i = 0; i < WORKERS; i++)
        {
            threads[i] = new std::thread{&TerrainWork, voxelData, &noise, i * SLICE_THICKNESS, SLICE_THICKNESS};
        }

        puts("");
        for (int i = 0; i < WORKERS; i++)
        {
            threads[i]->join();
            printf("\rProgress: %.1f", (float)i / WORKERS * 100);
        }

        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - it);
        printf("\nTIME: %lli\n\n", ms.count());
    }

    static void TerrainWork(uint8_t* voxelData, FastNoiseLite* noise, int z0, int SLICE_THICKNESS)
    {
        for (int z = z0; z < z0 + SLICE_THICKNESS; z++)
        {
            for (int y = 0; y < TERRAIN_AREA; y++)
            {
                for (int x = 0; x < TERRAIN_AREA; x++)
                {
                    uint8_t base = 0b11100000;
                    float v = (noise->GetNoise(x * 0.1f * TERRAIN_SCALE, y * 0.1f * TERRAIN_SCALE, z * 0.4f * TERRAIN_SCALE) + noise->GetNoise(x * 1.5f * TERRAIN_SCALE, y * 1.5f * TERRAIN_SCALE, z * 1.0f * TERRAIN_SCALE) * 0.03) / 1.03f;

                    int material = v <= 0.1;
                    if (material != MAT_AIR)
                    {
                        if (z > 0 && (voxel_mat(vox(x, y, z - 1)) == MAT_AIR))
                        {
                            material = MAT_GRASS;
                            float v = semiRandomFloat(x, y, z);
                            if (v < 0.08)
                            {
                                vox(x, y, z - 1) |= MAT_GRASS;

                                if (z > 1 && v < 0.008)
                                {
                                    vox(x, y, z - 2) |= MAT_FLOWER;
                                }
                            }
                        }
                        else
                        {
                            float v = noise->GetNoise(x * 1.5f * TERRAIN_SCALE, y * 1.5f * TERRAIN_SCALE, z * 1.f * TERRAIN_SCALE);
                            int32_t i = *((int32_t*)&v);
                            int32_t i2 = i | 9;
                            float v2 = *((float*)(&i2));
                            if (v2 < -0.15)
                            {
                                material = (i & 15) < 2 ? MAT_STONE2 : MAT_STONE3;
                            }
                        }
                    }

                    vox(x, y, z) = base | material;
                }
            }
        }
    }

    void UpdateVoxels(VkCommandBuffer commandBuffer, void* data, uint8_t section)
    {
    


        void* mappedData;
        vkMapMemory(device, voxelStagingBufferMemory, 0, 1024*1024*128, 0, &mappedData);
        memcpy(mappedData, (char*)voxelData + (section * 1024*1024*128), 1024*1024*128);
        vkUnmapMemory(device, voxelStagingBufferMemory);

        VkBufferImageCopy region = {};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = { 0, 0, 128 * section };
        region.imageExtent = { 1024, 1024, 128 };

        vkCmdCopyBufferToImage(commandBuffer, voxelStagingBuffer, voxelTexture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);


        void* mappedUpperData;
        vkMapMemory(device, voxelUpperStagingBufferMemory, 0, 32 * 32 * 8, 0, &mappedUpperData);
        memcpy(mappedUpperData, (char*)voxelUpperData + (section * 32 * 32 * 8), 32*32*4);
        vkUnmapMemory(device, voxelUpperStagingBufferMemory);

        VkBufferImageCopy upperRegion = {};
        upperRegion.bufferOffset = 0;
        upperRegion.bufferRowLength = 0;
        upperRegion.bufferImageHeight = 0;
        upperRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        upperRegion.imageSubresource.mipLevel = 0;
        upperRegion.imageSubresource.baseArrayLayer = 0;
        upperRegion.imageSubresource.layerCount = 1;
        upperRegion.imageOffset = { 0,0, section*4 };
        upperRegion.imageExtent = { 32,32,32 };

        vkCmdCopyBufferToImage(commandBuffer, voxelUpperStagingBuffer, voxelUpperTexture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &upperRegion);



        void* mappedVoxelChunkMapData;
        vkMapMemory(device, voxelChunkMapStagingBufferMemory, 0, 8*8*8*2, 0, &mappedVoxelChunkMapData);
        memcpy(mappedVoxelChunkMapData, voxelChunkMapData, 8*8*8*2);
        vkUnmapMemory(device, voxelChunkMapStagingBufferMemory);

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

        vkCmdCopyBufferToImage(commandBuffer, voxelChunkMapStagingBuffer, voxelChunkMapTexture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &voxelChunkMapRegion);
    }


    void createRaytracingPipeline()
    {
        auto raygenShaderCode = readFile("../rgen.spv");
        auto missShaderCode = readFile("../rmiss.spv");

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

        VkDescriptorSetLayoutBinding upperVoxelTexture = {};
        upperVoxelTexture.binding = 2;
        upperVoxelTexture.descriptorCount = 1;
        upperVoxelTexture.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        upperVoxelTexture.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        upperVoxelTexture.pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutBinding voxelTexture = {};
        voxelTexture.binding = 3;
        voxelTexture.descriptorCount = 1;
        voxelTexture.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        voxelTexture.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        voxelTexture.pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutBinding voxelChunkMapTexture = {};
        voxelChunkMapTexture.binding = 4;
        voxelChunkMapTexture.descriptorCount = 1;
        voxelChunkMapTexture.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        voxelChunkMapTexture.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        voxelChunkMapTexture.pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutBinding bindings[] = {storageImageBinding, uniformBinding, voxelTexture, upperVoxelTexture, voxelChunkMapTexture};

        VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {};
        layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutCreateInfo.bindingCount = 5;
        layoutCreateInfo.pBindings = bindings;
        layoutCreateInfo.pNext = nullptr;

        if (vkCreateDescriptorSetLayout(device, &layoutCreateInfo, nullptr, &raytracingDescriptorSetLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create raytracing descriptor set layout!");
        }

        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
        pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCreateInfo.setLayoutCount = 1;
        pipelineLayoutCreateInfo.pSetLayouts = &raytracingDescriptorSetLayout;

        if (vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &raytracingPipelineLayout) != VK_SUCCESS)
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

        PFN_vkCreateRayTracingPipelinesKHR vkCreateRayTracingPipelinesKHR = reinterpret_cast<PFN_vkCreateRayTracingPipelinesKHR>(vkGetDeviceProcAddr(device, "vkCreateRayTracingPipelinesKHR"));
        if (vkCreateRayTracingPipelinesKHR(device, VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &raytracingPipeline) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create raytracing pipeline!");
        }
        printf("PIPELINE LAYOUT %i\n", 1);
        vkDestroyShaderModule(device, raygenShaderModule, nullptr);
        vkDestroyShaderModule(device, missShaderModule, nullptr);


        // Pool

        VkDescriptorPoolSize storageImagePoolSize = {};
        storageImagePoolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        storageImagePoolSize.descriptorCount = 2;

        VkDescriptorPoolSize uboPoolSize = {};
        uboPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboPoolSize.descriptorCount = 2;

        VkDescriptorPoolSize upperVoxelPoolSize = {};
        upperVoxelPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        upperVoxelPoolSize.descriptorCount = 6;

        VkDescriptorPoolSize poolSizes[] = {storageImagePoolSize, uboPoolSize, upperVoxelPoolSize};

        VkDescriptorPoolCreateInfo poolCreateInfo = {};
        poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolCreateInfo.poolSizeCount = 3;
        poolCreateInfo.pPoolSizes = poolSizes;
        poolCreateInfo.maxSets = 50;

        if (vkCreateDescriptorPool(device, &poolCreateInfo, nullptr, &raytracingDescriptorPool) != VK_SUCCESS)
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
        
        if (vkAllocateDescriptorSets(device, &allocInfo, raytracingDescriptorSets.data()))
        {
            throw std::runtime_error("Failed to create raytracing descriptor set!");
        }

        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            

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
            bufferInfo.buffer = uniformBuffer;
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


            
            VkDescriptorImageInfo upperVoxelImageInfo = {};
            upperVoxelImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            upperVoxelImageInfo.imageView = voxelUpperImageView;
            upperVoxelImageInfo.sampler = voxelTextureSampler;

            VkWriteDescriptorSet upperVoxelDescriptorSet = {};
            upperVoxelDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            upperVoxelDescriptorSet.dstSet = raytracingDescriptorSets[i];
            upperVoxelDescriptorSet.dstBinding = 2;
            upperVoxelDescriptorSet.dstArrayElement = 0;
            upperVoxelDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            upperVoxelDescriptorSet.descriptorCount = 1;
            upperVoxelDescriptorSet.pImageInfo = &upperVoxelImageInfo;


            VkDescriptorImageInfo voxelImageInfo = {};
            voxelImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            voxelImageInfo.imageView = voxelImageView;
            voxelImageInfo.sampler = voxelTextureSampler;

            VkWriteDescriptorSet voxelDescriptorSet = {};
            voxelDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            voxelDescriptorSet.dstSet = raytracingDescriptorSets[i];
            voxelDescriptorSet.dstBinding = 3;
            voxelDescriptorSet.dstArrayElement = 0;
            voxelDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            voxelDescriptorSet.descriptorCount = 1;
            voxelDescriptorSet.pImageInfo = &voxelImageInfo;


            VkDescriptorImageInfo voxelChunkMapImageInfo = {};
            voxelChunkMapImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            voxelChunkMapImageInfo.imageView = voxelChunkMapImageView;
            voxelChunkMapImageInfo.sampler = voxelTextureSampler;

            VkWriteDescriptorSet voxelChunkMapDescriptorSet = {};
            voxelChunkMapDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            voxelChunkMapDescriptorSet.dstSet = raytracingDescriptorSets[i];
            voxelChunkMapDescriptorSet.dstBinding = 4;
            voxelChunkMapDescriptorSet.dstArrayElement = 0;
            voxelChunkMapDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            voxelChunkMapDescriptorSet.descriptorCount = 1;
            voxelChunkMapDescriptorSet.pImageInfo = &voxelChunkMapImageInfo;

            VkWriteDescriptorSet writeDescriptorSets[] = {writeStorageDescriptorSet, writeTransformDescriptorSet, upperVoxelDescriptorSet, voxelDescriptorSet, voxelChunkMapDescriptorSet};

            vkUpdateDescriptorSets(device, 5, writeDescriptorSets, 0, nullptr);
        }

    }

    void createGraphicsPipeline() {
        auto vertShaderCode = readFile("../vert.spv");
        auto fragShaderCode = readFile("../frag.spv");

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

        if (vkCreateDescriptorSetLayout(device, &layoutCreateInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create raytracing descriptor set layout!");
        }

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

        if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
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
        pipelineInfo.renderPass = renderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics pipeline!");
        }

        vkDestroyShaderModule(device, fragShaderModule, nullptr);
        vkDestroyShaderModule(device, vertShaderModule, nullptr);

         VkDescriptorPoolSize poolSize = {};
        poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSize.descriptorCount = 2;

        VkDescriptorPoolCreateInfo poolCreateInfo = {};
        poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolCreateInfo.poolSizeCount = 1;
        poolCreateInfo.pPoolSizes = &poolSize;
        poolCreateInfo.maxSets = 50;

        if (vkCreateDescriptorPool(device, &poolCreateInfo, nullptr, &descriptorPool) != VK_SUCCESS)
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
        
        if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()))
        {
            throw std::runtime_error("Failed to create raytracing descriptor set!");
        }

        imageSampler.resize(2);

        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {

            // image Sampler

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

            if (vkCreateSampler(device, &samplerCreateInfo, nullptr, &imageSampler[i]) != VK_SUCCESS)
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

            vkUpdateDescriptorSets(device, 1, &writeDescriptorSet, 0, nullptr);
        }

    }

    void createFramebuffers() {
        swapChainFramebuffers.resize(swapChainImageViews.size());

        for (size_t i = 0; i < swapChainImageViews.size(); i++) {
            VkImageView attachments[] = {
                swapChainImageViews[i]
            };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = swapChainExtent.width;
            framebufferInfo.height = swapChainExtent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create framebuffer!");
            }
        }
    }

    void createCommandPool() {
        QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

        if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create command pool!");
        }
    }

    void createCommandBuffers() {
        commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t) commandBuffers.size();

        if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }


        raytracingCommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo raytracingAllocInfo{};
        raytracingAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        raytracingAllocInfo.commandPool = commandPool;
        raytracingAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        raytracingAllocInfo.commandBufferCount = (uint32_t) commandBuffers.size();

        if (vkAllocateCommandBuffers(device, &raytracingAllocInfo, raytracingCommandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }

    void recordVoxelCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, uint8_t section)
    {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to begin recording command buffer!");
        }
        if (imageIndex == 0)
        {
            transitionImageLayout(commandBuffer, voxelChunkMapTexture, VK_FORMAT_R16_UINT, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1);
            transitionImageLayout(commandBuffer, voxelUpperTexture, VK_FORMAT_R8_UINT, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1);
            transitionImageLayout(commandBuffer, voxelTexture, VK_FORMAT_R8_UINT, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1);
            UpdateVoxels(commandBuffer, voxelData, section);
            transitionImageLayout(commandBuffer, voxelTexture, VK_FORMAT_R8_UINT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL, 1);
            transitionImageLayout(commandBuffer, voxelUpperTexture, VK_FORMAT_R8_UINT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL, 1);
            transitionImageLayout(commandBuffer, voxelChunkMapTexture, VK_FORMAT_R16_UINT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL, 1);
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

        PFN_vkCmdTraceRaysKHR vkCmdTraceRaysKHR = reinterpret_cast<PFN_vkCmdTraceRaysKHR>(vkGetDeviceProcAddr(device, "vkCmdTraceRaysKHR"));
        vkCmdTraceRaysKHR(
            commandBuffer,
            &raygenRegion,
            &missRegion,
            &hitRegion,
            &callableRegion,
            swapChainExtent.width,
            swapChainExtent.height,
            1
        );
        

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
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = swapChainExtent;

        VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

            VkViewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = (float) swapChainExtent.width;
            viewport.height = (float) swapChainExtent.height;
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

            VkRect2D scissor{};
            scissor.offset = {0, 0};
            scissor.extent = swapChainExtent;
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

    void createSyncObjects() {
        imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create synchronization objects for a frame!");
            }
        }
    }

    void drawFrame() {
        vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
        UpdateUBO(currentFrame);


        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain();
            return;
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        vkResetFences(device, 1, &inFlightFences[currentFrame]);

        vkResetCommandBuffer(commandBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
        vkResetCommandBuffer(raytracingCommandBuffers[(currentFrame - 1 + MAX_FRAMES_IN_FLIGHT) % MAX_FRAMES_IN_FLIGHT], 0);

        recordCommandBuffer(commandBuffers[currentFrame], imageIndex);
        recordVoxelCommandBuffer(raytracingCommandBuffers[(currentFrame - 1 + MAX_FRAMES_IN_FLIGHT) % MAX_FRAMES_IN_FLIGHT], (currentFrame - 1 + MAX_FRAMES_IN_FLIGHT) % MAX_FRAMES_IN_FLIGHT, section);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;


        std::vector<VkCommandBuffer> commands;
        commands.push_back(raytracingCommandBuffers[(currentFrame - 1 + MAX_FRAMES_IN_FLIGHT) % MAX_FRAMES_IN_FLIGHT]);
        commands.push_back(commandBuffers[currentFrame]);

        submitInfo.commandBufferCount = 2;
        submitInfo.pCommandBuffers = commands.data();

        VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = {swapChain};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        presentInfo.pImageIndices = &imageIndex;

        result = vkQueuePresentKHR(presentQueue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
            framebufferResized = false;
            recreateSwapChain();
        } else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
        if (currentFrame == 0)
        section = (section + 1) % 8;
    }

    VkCommandBuffer beginSingleTimeCommands() {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = commandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

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

        vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(graphicsQueue);

        vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
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
        VkDeviceSize bufferSize = sizeof(TransformUBO);

        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = bufferSize;
        bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(device, &bufferInfo, nullptr, &uniformBuffer) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create uniform buffer");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device, uniformBuffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        if (vkAllocateMemory(device, &allocInfo, nullptr, &uniformBufferMemory) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to allocate uniform buffer memory!");
        }

        vkBindBufferMemory(device, uniformBuffer, uniformBufferMemory, 0);

  
        vkMapMemory(device, uniformBufferMemory, 0, sizeof(TransformUBO), 0, &uniformBuffersMapped);
        memcpy(uniformBuffersMapped, &ubo, sizeof(TransformUBO));

        ubo.view = glm::mat4(1.0);
        ubo.proj = glm::perspective(glm::radians(70.0f), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 1000.0f);
        ubo.proj[1][1] *= -1;
        ubo.proj = glm::inverse(ubo.proj);
        
    }

    void onMouseMove(double xPos, double yPos) {
        mouseX = (float)xPos;
        mouseY = (float)yPos;
    }

    // Example of key press handler
    bool isKeyPressed(int key) {
        return glfwGetKey(window, key) == GLFW_PRESS;
    }

    // Example of mouse button press handler
    bool isMouseButtonPressed(int button) {
        return glfwGetMouseButton(window, button) == GLFW_PRESS;
    }

    // Function to hide the cursor
    void hideCursor() {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    // Function to show the cursor
    void showCursor() {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    void updateVoxelChunkMap(int modValue, int offset) {
        for (int z = 0; z < 8; z++) {
            for (int y = 0; y < 8; y++) {
                for (int x = 0; x < 8; x++) {
                    int index = z * 8 * 8 + y * 8 + x;
                    int chunk = voxelChunkMapData[index];
                    int chunkAxis = chunk % modValue;
                    voxelChunkMapData[index] = (chunk - chunkAxis) + (chunkAxis + offset) % modValue;
                }
            }
        }
    }

    bool firstMouse = true;

    void UpdateUBO(int currentFrame)
    {
        cameraPosition.z += cameraVelocity.z * deltaTime;
        cameraTargetPoint.z += cameraVelocity.z * deltaTime;

        glm::ivec3 intCameraPosition = glm::ivec3(cameraPosition) * -1;


        // floor
        bool is_grounded = false;
        for (int x = -1; x < 2; x++)
        {
            for (int y = -1; y < 2; y++)
            {
                is_grounded |= MAT_HAS_COLLISION(voxel_mat(vox(intCameraPosition.x + x, intCameraPosition.y + y, intCameraPosition.z + 30)));
            }
        }

        // ceiling
        bool is_ouch = false;
        for (int x = -1; x < 2; x++)
        {
            for (int y = -1; y < 2; y++)
            {
                is_ouch |= MAT_HAS_COLLISION(voxel_mat(vox(intCameraPosition.x + x, intCameraPosition.y + y, intCameraPosition.z - 6)));
            }
        }


        if (is_grounded)
        {
            cameraVelocity.z = 0;
        }
        else if (is_ouch)
        {
            // cameraVelocity.z = -fabsf(cameraVelocity.z) * 0.8;
        }
        else
        {
            // cameraVelocity.z -= 9.8 * 7 * deltaTime;
        }

        if (isMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT)) {
            // Hide the cursor and capture mouse movement
            hideCursor();

            float deltaTime = 0.1f;

            movementSpeed = isKeyPressed(GLFW_KEY_LEFT_SHIFT) ? 20 : 7.0;
            
            if (isKeyPressed(GLFW_KEY_S)) { cameraPosition += movementSpeed * deltaTime * glm::normalize(cameraTargetPoint - cameraPosition) * glm::vec3(1, 1, 0); }
            if (isKeyPressed(GLFW_KEY_W)) { cameraPosition -= movementSpeed * deltaTime * glm::normalize(cameraTargetPoint - cameraPosition) * glm::vec3(1, 1, 0); }
            if (isKeyPressed(GLFW_KEY_D)) { cameraPosition -= glm::normalize(glm::cross(cameraTargetPoint - cameraPosition, glm::vec3(0.0f, 0.0f, 1.0f))) * movementSpeed * deltaTime * glm::vec3(1, 1, 0); }
            if (isKeyPressed(GLFW_KEY_A)) { cameraPosition += glm::normalize(glm::cross(cameraTargetPoint - cameraPosition, glm::vec3(0.0f, 0.0f, 1.0f))) * movementSpeed * deltaTime * glm::vec3(1, 1, 0); }

            // Q and E for vertical movement
            if (isKeyPressed(GLFW_KEY_Q)) { cameraPosition.z -= movementSpeed * 2 * deltaTime; }
            if ((isKeyPressed(GLFW_KEY_SPACE) || isKeyPressed(GLFW_KEY_E)) && is_grounded)
            {
                cameraVelocity.z += 400 * deltaTime;
            }

            double currentMouseX, currentMouseY;
            glfwGetCursorPos(window, &currentMouseX, &currentMouseY);

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
            showCursor();
            firstMouse = true;
        }

        struct {
        float& pos;
        int lowerBound, upperBound, offset, modValue, shift;
    } checks[] = {
        {cameraPosition.x, -640, -340, 128, 8, 1},
        {cameraPosition.y, -640, -340, 128, 64, 8},
        {cameraPosition.z, -640, -340, 128, 512, 64}
    };


        ubo.view = glm::inverse(glm::lookAt(cameraPosition, cameraTargetPoint, glm::vec3(0.0f, 0.0f, 1.0f)));

        printf("\rPOSITION: %f %f %f\tDELTA TIME: %f\to: %i", cameraPosition.x, cameraPosition.y, cameraPosition.z, deltaTime, is_ouch);

        memcpy(uniformBuffersMapped, &ubo, sizeof(TransformUBO));

        for (auto& check : checks) {
            if (check.pos < check.lowerBound) {
                check.pos += check.offset;
                updateVoxelChunkMap(check.modValue, check.shift);
            } else if (check.pos > check.upperBound) {
                check.pos -= check.offset;
                updateVoxelChunkMap(check.modValue, check.modValue - check.shift);
            }
        
    }

        // if (cameraPosition.x < -640)
        // {
        //     cameraPosition.x += 128;
        //     for (int z = 0; z < 8; z++)
        //     {
        //         for (int y = 0; y < 8; y++)
        //         {
        //             for (int x = 0; x < 8; x++)
        //             {
        //                 int chunk = voxelChunkMapData[z*8*8 + y*8 + x];
        //                 int chunkX = chunk % 8;
        //                 voxelChunkMapData[z*8*8 + y*8 + x] = (chunk - chunkX) + (chunkX + 1) % 8;
        //             }
        //         }
        //     }
        // }
        // else if (cameraPosition.x > -340)
        // {
        //     cameraPosition.x -= 128;
        //     for (int z = 0; z < 8; z++)
        //     {
        //         for (int y = 0; y < 8; y++)
        //         {
        //             for (int x = 0; x < 8; x++)
        //             {
        //                 int chunk = voxelChunkMapData[z*8*8 + y*8 + x];
        //                 int chunkX = chunk % 8;
        //                 voxelChunkMapData[z*8*8 + y*8 + x] = (chunk - chunkX) + (chunkX + 7) % 8;
        //             }
        //         }
        //     }
        // }
        // else if (cameraPosition.y < -640)
        // {
        //     cameraPosition.y += 128;
        //     for (int z = 0; z < 8; z++)
        //     {
        //         for (int y = 0; y < 8; y++)
        //         {
        //             for (int x = 0; x < 8; x++)
        //             {
        //                 int chunk = voxelChunkMapData[z*8*8 + y*8 + x];
        //                 int chunkY = ((chunk - (chunk % 8)) % 64);
        //                 voxelChunkMapData[z*8*8 + y*8 + x] = (chunk - chunkY) + (chunkY + 8) % 64;
        //             }
        //         }
        //     }
        // }
        // else if (cameraPosition.y > -340)
        // {
        //     cameraPosition.y -= 128;
        //     for (int z = 0; z < 8; z++)
        //     {
        //         for (int y = 0; y < 8; y++)
        //         {
        //             for (int x = 0; x < 8; x++)
        //             {
        //                 int chunk = voxelChunkMapData[z*8*8 + y*8 + x];
        //                 int chunkY = ((chunk - (chunk % 8)) % 64);
        //                 printf("CHUNK Y: %i\n", chunkY);
        //                 voxelChunkMapData[z*8*8 + y*8 + x] = (chunk - chunkY) + (chunkY + 56) % 64;
        //             }
        //         }
        //     }
        // }
        // else if (cameraPosition.z < -640)
        // {
        //     cameraPosition.z += 128;
        //     for (int z = 0; z < 8; z++)
        //     {
        //         for (int y = 0; y < 8; y++)
        //         {
        //             for (int x = 0; x < 8; x++)
        //             {
        //                 int chunk = voxelChunkMapData[z*8*8 + y*8 + x];
        //                 int chunkZ = ((chunk - (chunk % 64)) % 512);
        //                 printf("CHUNK Y: %i\n", chunkZ);
        //                 voxelChunkMapData[z*8*8 + y*8 + x] = (chunk - chunkZ) + (chunkZ + 64) % 512;
        //             }
        //         }
        //     }
        // }
        // else if (cameraPosition.z > -340)
        // {
        //     cameraPosition.z -= 128;
        //     for (int z = 0; z < 8; z++)
        //     {
        //         for (int y = 0; y < 8; y++)
        //         {
        //             for (int x = 0; x < 8; x++)
        //             {
        //                 int chunk = voxelChunkMapData[z*8*8 + y*8 + x];
        //                 int chunkZ = ((chunk - (chunk % 64)) % 512);
        //                 printf("CHUNK Y: %i\n", chunkZ);
        //                 voxelChunkMapData[z*8*8 + y*8 + x] = (chunk - chunkZ) + (chunkZ + 448) % 512;
        //             }
        //         }
        //     }
        // }

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
        if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shader module!");
        }

        return shaderModule;
    }

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }

    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return availablePresentMode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        } else {
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);

            VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };

            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actualExtent;
        }
    }

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
        SwapChainSupportDetails details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
        }

        return details;
    }

    bool isDeviceSuitable(VkPhysicalDevice device) {
        QueueFamilyIndices indices = findQueueFamilies(device);

        bool extensionsSupported = checkDeviceExtensionSupport(device);

        bool swapChainAdequate = false;
        if (extensionsSupported) {
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        return indices.isComplete() && extensionsSupported && swapChainAdequate;
    }

    bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        for (const auto& extension : availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto& queueFamily : queueFamilies) {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
            }

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

            if (presentSupport) {
                indices.presentFamily = i;
            }

            if (indices.isComplete()) {
                break;
            }

            i++;
        }

        return indices;
    }

    std::vector<const char*> getRequiredExtensions() {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (enableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

    bool checkValidationLayerSupport() {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char* layerName : validationLayers) {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) {
                return false;
            }
        }

        return true;
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