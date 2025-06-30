#pragma once
#include "VulkanGlobals.h"
#include "VulkanUtils.h"
#include "../Core/Game/GameObject.h"
#include "../Core/Globals.h"

namespace Engine
{
	class VulkanCommand
	{
	private:
        void createCommandPool()
        {
            QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

            VkCommandPoolCreateInfo poolInfo{};
            poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

            if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to create graphics command pool!");
            }
        }

        void createCommandBuffers()
        {
            commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

            VkCommandBufferAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.commandPool = commandPool;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

            if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to allocate command buffers!");
            }
        }

        void createRayTracingCommandBuffers()
        {
            rayTracingCommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

            VkCommandBufferAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.commandPool = commandPool;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandBufferCount = (uint32_t)rayTracingCommandBuffers.size();

            if (vkAllocateCommandBuffers(device, &allocInfo, rayTracingCommandBuffers.data()) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to allocate command buffers!");
            }
        }

        void createCompositingCommandBuffers()
        {
            compositingCommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

            VkCommandBufferAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.commandPool = commandPool;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandBufferCount = (uint32_t)compositingCommandBuffers.size();

            if (vkAllocateCommandBuffers(device, &allocInfo, compositingCommandBuffers.data()) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to allocate command buffers!");
            }
        }

	public:
		VulkanCommand() {};

        void init()
        {
            createCommandPool();
        }

		void initCommandBuffers()
		{
			createCommandBuffers();
            createRayTracingCommandBuffers();
            createCompositingCommandBuffers();
		};

        void recordCommandBuffer(uint32_t gameObjectIndex, GameObject gameObject)
        {
            VkBuffer vertexBuffers[] = { gameManager.models[gameObject.model].vertexBuffer };
            VkDeviceSize offsets[] = { 0 };

            vkCmdBindVertexBuffers(commandBuffers[currentFrame], 0, 1, vertexBuffers, offsets);
            vkCmdBindIndexBuffer(commandBuffers[currentFrame], gameManager.models[gameObject.model].indexBuffer, 0, VK_INDEX_TYPE_UINT32);

            vkCmdBindDescriptorSets(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[currentFrame * gameManager.gameObjects.size() + gameObjectIndex], 0, nullptr);

            vkCmdDrawIndexed(commandBuffers[currentFrame], static_cast<uint32_t>(gameManager.models[gameObject.model].indices.size()), 1, 0, 0, 0);
        }
	};
}