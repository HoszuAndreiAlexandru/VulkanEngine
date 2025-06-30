#pragma once
#include "VulkanGlobals.h"
#include "VulkanUtils.h"

namespace Engine
{
	struct ImageHeader {
		uint32_t width;
		uint32_t height;
		uint32_t format;	// Vulkan format enum (VK_FORMAT_R8G8B8A8_SRGB)
		uint32_t dataSize;	// Total bytes of pixel data (width * height * 4)
		// Followed by: uint8_t pixelData[dataSize]
	};

	class VulkanTexture
	{
	private:
		void createTextureImage(std::string texturePath, VkImage& textureImage, VkDeviceMemory& textureImageMemory, ImVec2& textureSize, bool debug = true)
		{
			auto start = std::chrono::high_resolution_clock::now();
			int texWidth, texHeight, texChannels;
			stbi_uc* pixels = stbi_load(texturePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
			auto end1 = std::chrono::high_resolution_clock::now();
			auto duration1 = std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start).count();
			VkDeviceSize imageSize = texWidth * texHeight * 4;

			if (!pixels)
			{
				throw std::runtime_error("failed to load texture image!");
			}

			VkBuffer stagingBuffer;
			VkDeviceMemory stagingBufferMemory;
			createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

			void* data;
			vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
			memcpy(data, pixels, static_cast<size_t>(imageSize));
			vkUnmapMemory(device, stagingBufferMemory);

			stbi_image_free(pixels);

			createImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);
			textureSize = ImVec2(texWidth, texHeight);

			transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
			copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
			transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

			vkDestroyBuffer(device, stagingBuffer, nullptr);
			vkFreeMemory(device, stagingBufferMemory, nullptr);

			debug && std::cout << "\nTexture: " << texturePath << "\n    Disk load time: " << duration1 << " ms\n";
		};

		void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
		{
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
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;

			VkPipelineStageFlags sourceStage;
			VkPipelineStageFlags destinationStage;

			if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
			{
				barrier.srcAccessMask = 0;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

				sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			}
			else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			{
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
				destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			}
			else
			{
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
		};

		void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
		{
			VkCommandBuffer commandBuffer = beginSingleTimeCommands();

			VkBufferImageCopy region{};
			region.bufferOffset = 0;
			region.bufferRowLength = 0;
			region.bufferImageHeight = 0;
			region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.mipLevel = 0;
			region.imageSubresource.baseArrayLayer = 0;
			region.imageSubresource.layerCount = 1;
			region.imageOffset = { 0, 0, 0 };
			region.imageExtent = {
				width,
				height,
				1
			};

			vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

			endSingleTimeCommands(commandBuffer);
		};

		void createTextureImageView(VkImage& textureImage, VkImageView& VkImageView)
		{
			VkImageView = createImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
		};

		VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
		{
			VkImageViewCreateInfo viewInfo{};
			viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewInfo.image = image;
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewInfo.format = format;
			viewInfo.subresourceRange.aspectMask = aspectFlags;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.layerCount = 1;

			VkImageView imageView;
			if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create texture image view!");
			}

			return imageView;
		}

		void createTextureSampler(VkSampler& textureSampler)
		{
			VkPhysicalDeviceProperties properties{};
			vkGetPhysicalDeviceProperties(physicalDevice, &properties);

			VkSamplerCreateInfo samplerInfo{};
			samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			samplerInfo.magFilter = VK_FILTER_LINEAR;
			samplerInfo.minFilter = VK_FILTER_LINEAR;
			samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerInfo.anisotropyEnable = VK_TRUE;
			samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
			samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
			samplerInfo.unnormalizedCoordinates = VK_FALSE;
			samplerInfo.compareEnable = VK_FALSE;
			samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

			if (vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create texture sampler!");
			}
		};

		void writeImageCache(const std::string& cachePath, int width, int height, int format, const void* pixels, size_t size, int compression_level = 3)
		{
			std::ofstream file(cachePath, std::ios::binary);
			if (!file) throw std::runtime_error("Failed to open image cache for writing");

			file.write((char*)&width, sizeof(width));
			file.write((char*)&height, sizeof(height));
			file.write((char*)&format, sizeof(format));
			uint32_t dataSize = static_cast<uint32_t>(size);
			file.write((char*)&dataSize, sizeof(dataSize));
			file.write((char*)pixels, size);
		}

		bool readImageCache(const std::string& cachePath, int& width, int& height, int& format, std::vector<uint8_t>& pixels)
		{
			std::ifstream file(cachePath, std::ios::binary);
			if (!file) return false;

			// Read header
			uint32_t dataSize;
			file.read((char*)&width, sizeof(width));
			file.read((char*)&height, sizeof(height));
			file.read((char*)&format, sizeof(format));
			file.read((char*)&dataSize, sizeof(dataSize));

			pixels.resize(dataSize);
			file.read((char*)pixels.data(), dataSize);

			return file.good();
		}

		void createTextureImageWithCache(std::string texturePath, std::string textureName, VkImage& textureImage, VkDeviceMemory& textureImageMemory, ImVec2& textureSize, bool debug = true)
		{
			auto start = std::chrono::high_resolution_clock::now();
			int texWidth, texHeight, texChannels;

			std::string cachePath = "Resources/Cache/Textures/" + textureName + ".vkc";
			std::vector<uint8_t> pixelData;
			int format;

			if (!readImageCache(cachePath, texWidth, texHeight, format, pixelData))
			{
				stbi_uc* pixels = stbi_load(texturePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
				if (!pixels) throw std::runtime_error("failed to load texture!");

				size_t size = texWidth * texHeight * 4;
				pixelData.assign(pixels, pixels + size);
				stbi_image_free(pixels);

				format = VK_FORMAT_R8G8B8A8_SRGB;
				writeImageCache(cachePath, texWidth, texHeight, format, pixelData.data(), size);
			}

			VkDeviceSize imageSize = texWidth * texHeight * 4;
			auto end1 = std::chrono::high_resolution_clock::now();
			auto duration1 = std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start).count();

			VkBuffer stagingBuffer;
			VkDeviceMemory stagingBufferMemory;
			createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

			void* data;
			vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
			memcpy(data, pixelData.data(), static_cast<size_t>(imageSize));
			vkUnmapMemory(device, stagingBufferMemory);

			createImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);
			textureSize = ImVec2(texWidth, texHeight);

			transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
			copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
			transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

			vkDestroyBuffer(device, stagingBuffer, nullptr);
			vkFreeMemory(device, stagingBufferMemory, nullptr);

			debug&& std::cout << "\nTexture: " << texturePath << "\n    Disk load time: " << duration1 << " ms\n";
		}

	public:
		VulkanTexture() {};

		void init() {};

		void createVulkanTexture(std::string texturePath, std::string textureName)
		{
			GameTexture gameTexture;

			createTextureImageWithCache(texturePath, textureName, gameTexture.textureImage, gameTexture.textureImageMemory, gameTexture.size);
			createTextureImageView(gameTexture.textureImage, gameTexture.textureImageView);
			createTextureSampler(gameTexture.textureSampler);

			gameTexture.id = ImGui_ImplVulkan_AddTexture(
			gameTexture.textureSampler,
				gameTexture.textureImageView,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
			);

			//GameTexture gameTexture(textureImage, textureImageMemory, textureImageView, textureSampler);
			gameManager.textures[textureName] = gameTexture;
		};

		void createVulkanTextures(std::string folderPath)
		{
			try
			{
				for (const auto& entry : std::filesystem::directory_iterator(folderPath))
				{
					const std::filesystem::path filePath = entry.path();
					const std::string fileName = filePath.stem().string();      // File name without extension
					const std::string fileExtension = filePath.extension().string();

					if (fileExtension == ".jpg" || fileExtension == ".png" || fileExtension == ".jpeg")
					{
						this->createVulkanTexture(filePath.string(), fileName);
					}
				}
			}
			catch (const std::filesystem::filesystem_error& err)
			{
				std::cerr << "Filesystem error: " << err.what() << std::endl;
			}
		};
	};
}