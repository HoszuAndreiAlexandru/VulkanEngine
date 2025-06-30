#pragma once

#define GLFW_INCLUDE_VULKAN
#include <glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <chrono>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <array>
#include <optional>
#include <set>
#include <unordered_map>
#include <map>
#include <filesystem>
#include <mutex>
#include <thread>
#include <execution>
#include <future>

#include <Include/vulkan/vulkan_core.h>

#include <btBulletDynamicsCommon.h>

#include "../../_externals/imgui/imgui-master/backends/imgui_impl_vulkan.h"
#include "../../_externals/imgui/imgui-master/backends/imgui_impl_glfw.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

namespace fs = std::filesystem;

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete()
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;
    glm::vec3 normal;

    static VkVertexInputBindingDescription getBindingDescription()
    {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions()
    {
        std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

        attributeDescriptions[3].binding = 0;
        attributeDescriptions[3].location = 3;
        attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[3].offset = offsetof(Vertex, normal);

        return attributeDescriptions;
    }

    bool operator==(const Vertex& other) const
    {
        return pos == other.pos && color == other.color && texCoord == other.texCoord;
    }
};

namespace std
{
    template<> struct hash<Vertex>
    {
        /*
        size_t operator()(Vertex const& vertex) const
        {
            return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
        */

        size_t operator()(Vertex const& vertex) const {
            size_t h1 = hash<float>()(vertex.pos.x) ^ hash<float>()(vertex.pos.y) ^ hash<float>()(vertex.pos.z);
            size_t h2 = hash<float>()(vertex.normal.x) ^ hash<float>()(vertex.normal.y) ^ hash<float>()(vertex.normal.z);
            size_t h3 = hash<float>()(vertex.texCoord.x) ^ hash<float>()(vertex.texCoord.y);
            size_t h4 = hash<float>()(vertex.color.x) ^ hash<float>()(vertex.color.y) ^ hash<float>()(vertex.color.z);
            return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3);
        }
    };
}

struct Frustum
{
    glm::vec4 planes[6]; // Left, Right, Bottom, Top, Near, Far
};

#pragma region UBOs
struct UniformBufferObject
{
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
    alignas(16) glm::vec3 cameraPos;
    alignas(16) glm::vec3 cameraLookAt;
};
static_assert(sizeof(UniformBufferObject) % 16 == 0, "UniformBufferObject must be 16-byte aligned");

struct PushConstants
{
    int bvhNodeSize;
    int triangleSize;
    int instanceSize;
    int lightInstanceSize;
};

struct CameraUBO {
    alignas(16) glm::vec4 position;
    alignas(16) glm::vec4 direction;
    alignas(16) glm::vec4 up;
    alignas(16) glm::vec4 right;
    alignas(16) glm::vec4 screenCenter;
    alignas(16) glm::vec4 screenDimensions;

    float verticalFovRadians;
    float tanFov;
    float aspect;
    float pad;

    alignas(16) glm::mat4 projectionMatrix; // Projection matrix
    alignas(16) glm::mat4 viewMatrix;       // View matrix
};
static_assert(sizeof(CameraUBO) % 16 == 0, "CameraUBO must be 16-byte aligned");

struct Triangle
{
    glm::vec4 v0;
    glm::vec4 v1;
    glm::vec4 v2;
	glm::vec4 centroid; // Precomputed centroid for the triangle
};
static_assert(sizeof(Triangle) % 16 == 0, "Triangle must be 16-byte aligned");

struct AABB
{
    alignas(16) glm::vec3 min;
    float pad0;
    alignas(16) glm::vec3 max;
    float pad1;

    void expand(const glm::vec3& p)
    {
        min = glm::min(min, p);
        max = glm::max(max, p);
    }

    static AABB fromTriangle(const Triangle& tri)
    {
        AABB box;
        box.min = glm::min(glm::min(tri.v0, tri.v1), tri.v2);
        box.max = glm::max(glm::max(tri.v0, tri.v1), tri.v2);
        return box;
    }

    float surfaceArea() const
    {
        glm::vec3 d = max - min;
        return 2.f * (d.x * d.y + d.y * d.z + d.z * d.x);
    }
};

struct BVHNode
{
    glm::vec3 boundMin;
    float pad0;
    glm::vec3 boundMax;
    float pad1;
    int left = -1;
    int right = -1;
    int firstTriangle = -1;
    int triangleCount = 0;

    bool isLeaf() const
    {
        return triangleCount > 0;
    }

    void expand(const glm::vec3& p)
    {
        boundMin = glm::min(boundMin, p);
        boundMax = glm::max(boundMax, p);
    }

    float surfaceArea() const
    {
        glm::vec3 d = boundMax - boundMin;
        return 2.f * (d.x * d.y + d.y * d.z + d.z * d.x);
    }
};
static_assert(sizeof(BVHNode) % 16 == 0, "BVHNode must be 16-byte aligned");

struct BVHInstance
{
    glm::mat4 modelMatrix;
    glm::mat4 inverseModelMatrix;
    alignas(16) int bvhRootNodeIndex;
    int triangleOffset;
    int triangleCount;
    int pad0;
};
static_assert(sizeof(BVHInstance) % 16 == 0, "BVHInstance must be 16-byte aligned");

struct LightInstance
{
    alignas(16) glm::vec3 position;
    float pad0;

    alignas(16) glm::vec3 color;
    float pad1;

    float intensity;
    float radius;
    float pad2;
    float pad3;
};
static_assert(sizeof(LightInstance) % 16 == 0, "LightInstance must be 16-byte aligned");
#pragma endregion