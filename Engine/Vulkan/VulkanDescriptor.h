#pragma once
#include "../Core/Globals.h"
#include "VulkanGlobals.h"

namespace Engine
{
    class VulkanDescriptor
    {
    private:
        uint32_t descriptorCount = maxNumberOfObjects;

        #pragma region Rasterization
        void createDescriptorPool()
        {
            std::array<VkDescriptorPoolSize, 3> poolSizes{};
            poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * descriptorCount * 2);
            poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * descriptorCount * 2);
            poolSizes[2] = { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * descriptorCount * 2) };

            VkDescriptorPoolCreateInfo poolInfo{};
            poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
            poolInfo.pPoolSizes = poolSizes.data();
            poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * descriptorCount * 2);

            if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to create descriptor pool!");
            }
        };

        void createDescriptorSetLayout()
        {
            std::array<VkDescriptorSetLayoutBinding, 3> bindings{};

            bindings[0].binding = 0;
            bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            bindings[0].descriptorCount = 1;
            bindings[0].stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
            bindings[0].pImmutableSamplers = nullptr;

            bindings[1].binding = 1;
            bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            bindings[1].descriptorCount = 1;
            bindings[1].stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
            bindings[1].pImmutableSamplers = nullptr;

            bindings[2].binding = 2;
            bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            bindings[2].descriptorCount = 1;
            bindings[2].stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
            bindings[2].pImmutableSamplers = nullptr;

            VkDescriptorSetLayoutCreateInfo layoutInfo{};
            layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
            layoutInfo.pBindings = bindings.data();

            if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to create descriptor set layout!");
            }
        }

        void createDescriptorSets()
        {
            std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT * descriptorCount, descriptorSetLayout);

            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool = descriptorPool;
            allocInfo.descriptorSetCount = static_cast<uint32_t>(layouts.size());
            allocInfo.pSetLayouts = layouts.data();

            descriptorSets.resize(layouts.size());
            VkResult result = vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data());
            if (result != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to allocate descriptor sets!");
            }
        };
        #pragma endregion

        #pragma region Compute raytracing
        void createComputeRaytracingDescriptorSetLayout()
        {
            for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
            {
                std::array<VkDescriptorSetLayoutBinding, 6> bindings{};

                // Binding 0: Storage Image
                bindings[0].binding = 0;
                bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                bindings[0].descriptorCount = 1;
                bindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
                bindings[0].pImmutableSamplers = nullptr;

                // Binding 1: Uniform Buffer (Camera)
                bindings[1].binding = 1;
                bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                bindings[1].descriptorCount = 1;
                bindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
                bindings[1].pImmutableSamplers = nullptr;

                // Binding 2: BVH buffer
                bindings[2].binding = 2;
                bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                bindings[2].descriptorCount = 1;
                bindings[2].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
                bindings[2].pImmutableSamplers = nullptr;

                // Binding 3: Triangle buffer
                bindings[3].binding = 3;
                bindings[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                bindings[3].descriptorCount = 1;
                bindings[3].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
                bindings[3].pImmutableSamplers = nullptr;

                // Binding 4: Instance buffer
                bindings[4].binding = 4;
                bindings[4].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                bindings[4].descriptorCount = 1;
                bindings[4].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
                bindings[4].pImmutableSamplers = nullptr;

                // Binding 5: Light buffer
                bindings[5].binding = 5;
                bindings[5].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                bindings[5].descriptorCount = 1;
                bindings[5].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
                bindings[5].pImmutableSamplers = nullptr;

                // Create layout
                VkDescriptorSetLayoutCreateInfo layoutInfo{};
                layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
                layoutInfo.pBindings = bindings.data();

                if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &rayTracingDescriptorSetLayout[i]) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create descriptor set layout!");
                }
            }
        }

        void createComputeRaytracingDescriptorPool()
        {
            std::array<VkDescriptorPoolSize, 3> poolSizes{};
            poolSizes[0] = { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 10 };
            poolSizes[1] = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10 };
            poolSizes[2] = { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 40 };

            VkDescriptorPoolCreateInfo poolInfo{};
            poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
            poolInfo.pPoolSizes = poolSizes.data();
            poolInfo.poolSizeCount = sizeof(poolSizes) / sizeof(poolSizes[0]);
            poolInfo.maxSets = 2;

            if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &rayTracingDescriptorPool) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to create raytracing descriptor pool!");
            }
        }

		void createComputeRayTracingDescriptorSet()
		{
            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool = rayTracingDescriptorPool;
            allocInfo.descriptorSetCount = 2;
            allocInfo.pSetLayouts = rayTracingDescriptorSetLayout;

            vkAllocateDescriptorSets(device, &allocInfo, rayTracingDescriptorSet);
		}
        #pragma endregion

        #pragma region Compositing
        void createCompositingDescriptorPool()
        {
            std::array<VkDescriptorPoolSize, 1> poolSizes{};
            poolSizes[0] = { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 10 };

            VkDescriptorPoolCreateInfo poolInfo{};
            poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
            poolInfo.pPoolSizes = poolSizes.data();
            poolInfo.poolSizeCount = sizeof(poolSizes) / sizeof(poolSizes[0]);
            poolInfo.maxSets = 2;

            if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &compositingDescriptorPool) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to create compositing descriptor pool!");
            }
        }

        void createCompositingDescriptorSet()
        {
            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool = compositingDescriptorPool;
            allocInfo.descriptorSetCount = 2;
            allocInfo.pSetLayouts = compositingDescriptorSetLayout;

            vkAllocateDescriptorSets(device, &allocInfo, compositingDescriptorSet);
        }
        #pragma endregion

    public:
        VulkanDescriptor() {};

        void init()
        {
            createDescriptorSetLayout();
            createDescriptorPool();
            createDescriptorSets();

            //createComputeRaytracingDescriptorSetLayout();
            createComputeRaytracingDescriptorPool();
            createComputeRayTracingDescriptorSet();

            //createCompositingDescriptorPool();
            //createCompositingDescriptorSet();
        };

        VkDescriptorSet getDescriptorSet(size_t index) const
        {
            return descriptorSets.at(index);
        }

        void preallocateDescriptorSets()
        {
            for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
            {
                for (size_t j = 0; j < descriptorCount; j++)
                {
                    VkDescriptorBufferInfo bufferInfo{};
                    bufferInfo.buffer = uniformBuffers[i * descriptorCount + j];
                    bufferInfo.offset = 0;
                    bufferInfo.range = sizeof(UniformBufferObject);

                    VkDescriptorImageInfo imageInfo{};
                    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

                    try
                    {
                        imageInfo.imageView = gameManager.textures.at("default").textureImageView;
                        imageInfo.sampler = gameManager.textures.at("default").textureSampler;
                    }
                    catch (exception e)
                    {
                        imageInfo.imageView = gameManager.textures.at("default").textureImageView;
                        imageInfo.sampler = gameManager.textures.at("default").textureSampler;
                    }

                    VkDescriptorImageInfo raytracedImageInfo{};
                    raytracedImageInfo.imageView = raytracingImageView;
                    raytracedImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

                    std::array<VkWriteDescriptorSet, 3> descriptorWrites{};

                    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorWrites[0].dstSet = descriptorSets[i * descriptorCount + j];
                    descriptorWrites[0].dstBinding = 0;
                    descriptorWrites[0].dstArrayElement = 0;
                    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    descriptorWrites[0].descriptorCount = 1;
                    descriptorWrites[0].pBufferInfo = &bufferInfo;

                    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorWrites[1].dstSet = descriptorSets[i * descriptorCount + j];
                    descriptorWrites[1].dstBinding = 1;
                    descriptorWrites[1].dstArrayElement = 0;
                    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    descriptorWrites[1].descriptorCount = 1;
                    descriptorWrites[1].pImageInfo = &imageInfo;

                    descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorWrites[2].dstSet = descriptorSets[i * descriptorCount + j];
                    descriptorWrites[2].dstBinding = 2;
                    descriptorWrites[2].dstArrayElement = 0;
                    descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                    descriptorWrites[2].descriptorCount = 1;
                    descriptorWrites[2].pImageInfo = &raytracedImageInfo;

                    vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
                }
            }
        }

        void updateDescriptorSet(size_t index, VkBuffer buffer1, VkDeviceSize bufferSize, VkImageView imageView, VkSampler sampler)
        {
            if (index >= descriptorCount || (descriptorCount + index) >= descriptorSets.size()) return;

            VkDescriptorBufferInfo bufferInfo1{};
            bufferInfo1.buffer = buffer1;
            bufferInfo1.offset = 0;
            bufferInfo1.range = bufferSize;

            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = imageView;
            imageInfo.sampler = sampler;

            VkDescriptorImageInfo raytracedImageInfo{};
            raytracedImageInfo.imageView = raytracingImageView;
            raytracedImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

            std::array<VkWriteDescriptorSet, 3> descriptorWrites{};

            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = descriptorSets[index];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo1;

            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = descriptorSets[index];
            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pImageInfo = &imageInfo;

            descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[2].dstSet = descriptorSets[index];
            descriptorWrites[2].dstBinding = 2;
            descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            descriptorWrites[2].descriptorCount = 1;
            descriptorWrites[2].pImageInfo = &raytracedImageInfo;

            std::array<VkWriteDescriptorSet, 3> descriptorWrites2{};

            descriptorWrites2[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites2[0].dstSet = descriptorSets[descriptorCount + index];
            descriptorWrites2[0].dstBinding = 0;
            descriptorWrites2[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites2[0].descriptorCount = 1;
            descriptorWrites2[0].pBufferInfo = &bufferInfo1;

            descriptorWrites2[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites2[1].dstSet = descriptorSets[descriptorCount + index];
            descriptorWrites2[1].dstBinding = 1;
            descriptorWrites2[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites2[1].descriptorCount = 1;
            descriptorWrites2[1].pImageInfo = &imageInfo;

            descriptorWrites2[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites2[2].dstSet = descriptorSets[index];
            descriptorWrites2[2].dstBinding = 2;
            descriptorWrites2[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            descriptorWrites2[2].descriptorCount = 1;
            descriptorWrites2[2].pImageInfo = &raytracedImageInfo;

            vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
            vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites2.size()), descriptorWrites2.data(), 0, nullptr);
        }

        void updateRayTracingDescriptorSet()
        {
            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageView = raytracingImageView;
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

            VkDescriptorBufferInfo cameraBufferInfo{};
            cameraBufferInfo.buffer = cameraBuffer;
            cameraBufferInfo.offset = 0;
            cameraBufferInfo.range = sizeof(CameraUBO);

            VkDescriptorBufferInfo bvhBufferInfo{};
            bvhBufferInfo.buffer = bvhBuffer;
            bvhBufferInfo.offset = 0;
            bvhBufferInfo.range = sizeof(uint32_t) + bvhNodes.size() * sizeof(BVHInstance); // VK_WHOLE_SIZE

            VkDescriptorBufferInfo triangleBufferInfo{};
            triangleBufferInfo.buffer = triangleBuffer;
            triangleBufferInfo.offset = 0;
            triangleBufferInfo.range = sizeof(uint32_t) + bvhTriangles.size() * sizeof(Triangle);

            VkDescriptorBufferInfo instancesBufferInfo{};
            instancesBufferInfo.buffer = instanceBuffer;
            instancesBufferInfo.offset = 0;
            instancesBufferInfo.range = sizeof(uint32_t) + bvhInstances.size() * sizeof(BVHInstance);

            VkDescriptorBufferInfo lightInstancesBufferInfo{};
            lightInstancesBufferInfo.buffer = lightBuffer;
            lightInstancesBufferInfo.offset = 0;
            lightInstancesBufferInfo.range = VK_WHOLE_SIZE;

            std::array<VkWriteDescriptorSet, 6> descriptorWrites;
            {
                // Binding 0: storage image (output target)
                {
                    descriptorWrites[0] = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
                    descriptorWrites[0].dstSet = rayTracingDescriptorSet[currentFrame];
                    descriptorWrites[0].dstBinding = 0;
                    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                    descriptorWrites[0].descriptorCount = 1;
                    descriptorWrites[0].pImageInfo = &imageInfo;
                }

                // Binding 1: camera uniform buffer
                {
                    descriptorWrites[1] = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
                    descriptorWrites[1].dstSet = rayTracingDescriptorSet[currentFrame];
                    descriptorWrites[1].dstBinding = 1;
                    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    descriptorWrites[1].descriptorCount = 1;
                    descriptorWrites[1].pBufferInfo = &cameraBufferInfo;
                }

                // Binding 2: BVH nodes buffer
                {
                    descriptorWrites[2] = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
                    descriptorWrites[2].dstSet = rayTracingDescriptorSet[currentFrame];
                    descriptorWrites[2].dstBinding = 2;
                    descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    descriptorWrites[2].descriptorCount = 1;
                    descriptorWrites[2].pBufferInfo = &bvhBufferInfo;
                }

                // Binding 3: triangles buffer
                {
                    descriptorWrites[3] = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
                    descriptorWrites[3].dstSet = rayTracingDescriptorSet[currentFrame];
                    descriptorWrites[3].dstBinding = 3;
                    descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    descriptorWrites[3].descriptorCount = 1;
                    descriptorWrites[3].pBufferInfo = &triangleBufferInfo;
                }

                // Binding 4: BVH instances buffer
                {
                    descriptorWrites[4] = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
                    descriptorWrites[4].dstSet = rayTracingDescriptorSet[currentFrame];
                    descriptorWrites[4].dstBinding = 4;
                    descriptorWrites[4].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    descriptorWrites[4].descriptorCount = 1;
                    descriptorWrites[4].pBufferInfo = &instancesBufferInfo;
                }

                // Binding 5: Light instances buffer
                {
                    descriptorWrites[5] = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
                    descriptorWrites[5].dstSet = rayTracingDescriptorSet[currentFrame];
                    descriptorWrites[5].dstBinding = 5;
                    descriptorWrites[5].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    descriptorWrites[5].descriptorCount = 1;
                    descriptorWrites[5].pBufferInfo = &lightInstancesBufferInfo;
                }
            }

            vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }

        void updateCompositingDescriptorSet()
        {
            // Binding 0: storage image (output target)
            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageView = raytracingImageView;
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

			// Binding 1: rasterized storage image
			VkDescriptorImageInfo rasterizedImageInfo{};
			rasterizedImageInfo.imageView = swapChainImageViews[currentFrame];
			rasterizedImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

            std::array<VkWriteDescriptorSet, 2> descriptorWrites;
            {
                // Storage Image
                {
                    descriptorWrites[0] = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
                    descriptorWrites[0].dstSet = compositingDescriptorSet[currentFrame];
                    descriptorWrites[0].dstBinding = 0;
                    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                    descriptorWrites[0].descriptorCount = 1;
                    descriptorWrites[0].pImageInfo = &imageInfo;
                }

				// Rasterized Image
				{
					descriptorWrites[1] = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
					descriptorWrites[1].dstSet = compositingDescriptorSet[currentFrame];
					descriptorWrites[1].dstBinding = 1;
					descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
					descriptorWrites[1].descriptorCount = 1;
					descriptorWrites[1].pImageInfo = &rasterizedImageInfo;
				}
            }

            vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }
    };
}