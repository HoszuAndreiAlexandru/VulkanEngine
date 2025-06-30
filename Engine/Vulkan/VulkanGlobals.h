#pragma once
#include "VulkanTypes.h"

bool debugVulkan = false;

inline bool enableValidationLayers = true;
inline std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };

inline VkInstance vkInstance;
inline VkDebugUtilsMessengerEXT debugMessenger;
inline VkSurfaceKHR surface;

VkQueryPool timestampQueryPool;
VkPhysicalDeviceProperties deviceProperties;
uint64_t timestamps[4];

inline VkPhysicalDevice physicalDevice;
inline VkDevice device;
inline std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
inline VkQueue graphicsQueue;
inline VkQueue presentQueue;
//inline VkQueue computeQueue;

inline const int MAX_FRAMES_IN_FLIGHT = 2;

inline std::vector<VkSemaphore> imageAvailableSemaphores;
inline std::vector<VkSemaphore> renderFinishedSemaphores;
inline std::vector<VkFence> inFlightFences;

inline uint32_t imageIndex;

#pragma region Rasterization
inline VkSwapchainKHR swapChain;
inline std::vector<VkImage> swapChainImages;
inline VkFormat swapChainImageFormat;
inline VkExtent2D swapChainExtent;
inline std::vector<VkImageView> swapChainImageViews;
inline std::vector<VkFramebuffer> swapChainFramebuffers;

inline VkImage depthImage;
inline VkDeviceMemory depthImageMemory;
inline VkImageView depthImageView;

inline VkRenderPass renderPass;
inline VkDescriptorSetLayout descriptorSetLayout;
inline VkPipelineLayout pipelineLayout;
inline VkPipeline graphicsPipeline;

inline VkCommandPool commandPool;

inline std::vector<VkCommandBuffer> commandBuffers;

inline std::vector<VkBuffer> uniformBuffers;
inline std::vector<VkDeviceMemory> uniformBuffersMemory;
inline std::vector<void*> uniformBuffersMapped;

inline VkDescriptorPool descriptorPool;
inline std::vector<VkDescriptorSet> descriptorSets;
#pragma endregion

#pragma region Raytracing compute
inline VkDescriptorSetLayout rayTracingDescriptorSetLayout[MAX_FRAMES_IN_FLIGHT];
inline VkPipelineLayout rayTracingPipelineLayout;
inline VkPipeline rayTracingPipeline;

inline VkDescriptorPool rayTracingDescriptorPool;
inline VkDescriptorSet rayTracingDescriptorSet[MAX_FRAMES_IN_FLIGHT];

inline std::vector<VkCommandBuffer> rayTracingCommandBuffers;

// Descriptor sets
// Push constants
const uint32_t integerByteSize = sizeof(int);
const uint32_t computePushConstantCountInteger = 4; // Number of push constants you want to use

// 1: raytracing image
inline VkImage raytracingImage;
inline VkDeviceMemory raytracingImageMemory;
inline VkImageView raytracingImageView;

// 2: camera data
CameraUBO cameraUBO{};
VkBuffer cameraBuffer;
VkDeviceMemory cameraBufferMemory;

// 3: BVH data
std::vector<BVHNode> bvhNodes;
VkBuffer bvhBuffer;
VkDeviceMemory bvhBufferMemory;
// Staging buffer
VkBuffer bvhStagingBuffer;
VkDeviceMemory bvhStagingBufferMemory;

// 4: triangle data
std::vector<Triangle> bvhTriangles;
VkBuffer triangleBuffer;
VkDeviceMemory triangleBufferMemory;
// Staging buffer
VkBuffer triangleStagingBuffer;
VkDeviceMemory triangleStagingBufferMemory;

// 5: instance data
std::vector<BVHInstance> bvhInstances;
VkBuffer instanceBuffer;
VkDeviceMemory instanceBufferMemory;
// Staging buffer
VkBuffer instanceStagingBuffer;
VkDeviceMemory instanceStagingBufferMemory;

// 6: light sources
std::vector<LightInstance> lightInstances;
VkBuffer lightBuffer;
VkDeviceMemory lightBufferMemory;
// Staging buffer
VkBuffer lightStagingBuffer;
VkDeviceMemory lightStagingBufferMemory;

// 7: texture data
std::vector<VkDescriptorSet> textures;
#pragma endregion

#pragma region Compositing
inline VkDescriptorSetLayout compositingDescriptorSetLayout[MAX_FRAMES_IN_FLIGHT];
inline VkPipelineLayout compositingPipelineLayout;
inline VkPipeline compositingPipeline;

inline VkDescriptorPool compositingDescriptorPool;
inline VkDescriptorSet compositingDescriptorSet[MAX_FRAMES_IN_FLIGHT];

inline std::vector<VkCommandBuffer> compositingCommandBuffers;

// Descriptor sets
// 1: raytraced image from above raytracing compute variables

/*
// 2: rasterized image
inline VkImage rasterizedImage;
inline VkDeviceMemory rasterizedImageMemory;
inline VkImageView rasterizedImageView;

// 3: compositing image
inline VkImage compositedImage;
inline VkDeviceMemory compositedImageMemory;
inline VkImageView compositedImageView;
*/
#pragma endregion

#pragma region ImGui
inline ImGui_ImplVulkanH_Window g_MainWindowData;
inline ImGui_ImplVulkanH_Window* wd = &g_MainWindowData;
#pragma endregion