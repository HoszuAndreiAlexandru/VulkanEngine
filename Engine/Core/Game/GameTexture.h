#pragma once
#include <vulkan/vulkan.h>

namespace Engine
{
	class GameTexture
	{
	public:
		VkImage textureImage;
		VkDeviceMemory textureImageMemory;
		VkImageView textureImageView;
		VkSampler textureSampler;
		VkDescriptorSet id;
		ImVec2 size;

		GameTexture() {};
	};
};