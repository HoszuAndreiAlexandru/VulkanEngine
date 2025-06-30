#pragma once
#include <vulkan/vulkan.h>
#include "../Vulkan/VulkanInstance.h"
#include "../Vulkan/VulkanDeviceManager.h"
#include "../Vulkan/VulkanSwapChain.h"
#include "../Vulkan/VulkanPipeline.h"
#include "../Vulkan/VulkanCommand.h"
#include "../Vulkan/VulkanTexture.h"
#include "../Vulkan/VulkanModel.h"
#include "../Vulkan/VulkanUniform.h"
#include "../Vulkan/VulkanDescriptor.h"
#include "../Vulkan/VulkanSync.h"

#include "Game/GameManager.h"
#include "Globals.h"

#include "UI/Button.h"
#include "UI/Image.h"
#include "UI/Text.h"

using namespace std;

namespace Engine
{
	class VulkanRenderer
	{
	private:
		VulkanInstance instance;
		VulkanDeviceManager deviceManager;
		VulkanSwapChain swapChainManager;
		VulkanPipeline pipeline;
		VulkanCommand commandManager;
		VulkanTexture textureManager;
		VulkanModel modelManager;
		VulkanUniform uniformManager;
		VulkanDescriptor descriptorManager;
		VulkanSync syncManager;

        Window* window;

        #pragma region Asset loading
		void loadTextures(string path)
		{
            this->textureManager.createVulkanTextures(path);
		};

		void loadModels(string path)
		{
            this->modelManager.createVulkanModels(path);
		};

		void loadScenes(string path)
		{
            // TODO
		};
        #pragma endregion

        void loadDefaultScene()
        {
            GameObject bunny0;
            bunny0.setPosition(glm::vec3(0.001f, 1, 0));
            bunny0.CreateRigidBody(gameManager.models["stanford-bunny"]);
            gameManager.gameObjects["bunny0"] = bunny0;
            gameManager.gameScenes[gameManager.currentScene].addGameObject(bunny0);

            GameObject bunny1;
            bunny1.setPosition(glm::vec3(0.002f, 2, 0));
            bunny1.CreateRigidBody(gameManager.models["teapot"]);
            gameManager.gameObjects["bunny"] = bunny1;
            gameManager.gameScenes[gameManager.currentScene].addGameObject(bunny1);

            
            GameObject bunny2;
            bunny2.setPosition(glm::vec3(0.003f, 3, 0));
            bunny2.CreateRigidBody(gameManager.models["teapot"]);
            gameManager.gameObjects["bunny2"] = bunny2;
            gameManager.gameScenes[gameManager.currentScene].addGameObject(bunny2);

            GameObject bunny3;
            bunny3.setPosition(glm::vec3(0.001f, 4, 0));
            bunny3.CreateRigidBody(gameManager.models["teapot"]);
            gameManager.gameObjects["bunny3"] = bunny3;
            gameManager.gameScenes[gameManager.currentScene].addGameObject(bunny3);
			//descriptorManager.updateDescriptorSet(0, uniformBuffers[0], sizeof(UniformBufferObject), gameManager.textures["default"].textureImageView, gameManager.textures["default"].textureSampler);
            
            GameObject bunny4;
            bunny4.setPosition(glm::vec3(0.002f, 5, 0));
            bunny4.CreateRigidBody(gameManager.models["stanford-bunny"]);
            gameManager.gameObjects["bunny4"] = bunny4;
            gameManager.gameScenes[gameManager.currentScene].addGameObject(bunny4);


            GameObject quad(true);
			quad.isStatic = true;
			quad.isTerrain = true;
			quad.setPosition(glm::vec3(0, 0, 0));
			quad.scale = glm::vec3(1000, 1000, 1000);
            quad.CreateRigidBody(gameManager.models["quad"]);
            quad.onUpdate = [quad]()
            {
                    //btTransform transform = quad->rigidBody->getWorldTransform();
                    //btVector3 position = transform.getOrigin();
                    //glm::vec3 glmPosition(position.x(), position.y() - 0.01f, position.z());
					//quad->setPosition(glmPosition);
			};
			gameManager.gameObjects["quad"] = quad;
			gameManager.gameScenes[gameManager.currentScene].addGameObject(quad);
            //descriptorManager.updateDescriptorSet(1, uniformBuffers[0], sizeof(UniformBufferObject), gameManager.textures["default"].textureImageView, gameManager.textures["default"].textureSampler);

            //GameObject sun(&gameManager.models["viking_room"], true);
            //sun.isStatic = true;
            //sun.isTerrain = true;
            //sun.isLight = true;
            //sun.model = "bunny";
            //sun.setPosition(glm::vec3(100, 100, 100));
            //gameManager.gameScenes[gameManager.currentScene].addGameObject(sun);
            //descriptorManager.updateDescriptorSet(2, uniformBuffers[0], sizeof(UniformBufferObject), gameManager.textures["default"].textureImageView, gameManager.textures["default"].textureSampler);

			gameManager.gameCameras[gameManager.currentCamera].position = glm::vec3(2, 1, 0);
            //gameManager.gameCameras[gameManager.currentCamera].position = glm::vec3(1, 1, 1);
            gameManager.gameCameras[gameManager.currentCamera].lookAt = glm::vec3(0, 0, 0);

            UiText* fpsText = new UiText();
            fpsText->posX = 0;
            fpsText->posY = 0;
            fpsText->textColor = glm::vec3(1, 0, 0);
            fpsText->text = "FPS: 0";
            gameManager.canvases[gameManager.currentCanvas].uiElements["fpsText"] = fpsText;

            /*
            UiButton* button1 = new UiButton();
			button1->posX = 0;
			button1->posY = 0;
            button1->textColor = glm::vec3(1, 0, 0);
			button1->text = "Button 1";
			//button1.onClick = []() { std::cout << "Button 1 clicked!" << std::endl; };
			gameManager.canvases[gameManager.currentCanvas].uiElements["button1"] = button1;
            
            UiImage* image1 = new UiImage();
            image1->posX = 100;
			image1->posY = 100;
			image1->size = ImVec2(100, 100);
            gameManager.canvases[gameManager.currentCanvas].uiElements["image1"] = image1;
            */
        }

        void waitForPreviousFrame()
        {
            /// Wait for previous frame
            {
                vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
            }

            /// Acquire next image from swap chain
            {
                VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

                if (result == VK_ERROR_OUT_OF_DATE_KHR) {
                    swapChainManager.recreateSwapChain(instance.window);
                    return;
                }
                else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
                    throw std::runtime_error("failed to acquire swap chain image!");
                }
            }

            /// Reset fences
            {
                vkResetFences(device, 1, &inFlightFences[currentFrame]);
            }

            /// Reset command buffers
            {
                if (commandBuffers.size() == 0 || rayTracingCommandBuffers.size() == 0)
                {
                    return;
                }
                else
                {
                    vkResetCommandBuffer(commandBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
                    vkResetCommandBuffer(rayTracingCommandBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
                }
            }
        }

        void imageBarrierToGeneral(VkCommandBuffer commandBuffer)
        {
            VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
            barrier.srcAccessMask = 0;  // Because VK_IMAGE_LAYOUT_UNDEFINED implies no accesses
            barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = raytracingImage;
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;

            vkCmdPipelineBarrier(
                commandBuffer,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,    // srcStage
                VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, // dstStage
                0,
                0, nullptr,
                0, nullptr,
                1, &barrier
            );
        }

        void imageBarrierToReadOnly(VkCommandBuffer commandBuffer)
        {
            VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = raytracingImage;
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;

            vkCmdPipelineBarrier(
                commandBuffer,
                VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                0,
                0, nullptr,
                0, nullptr,
                1, &barrier
            );
        }

        void bufferBarrier(VkCommandBuffer commandBuffer, VkBuffer buffer)
        {
            VkBufferMemoryBarrier bufferBarrier{};
            bufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
            bufferBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            bufferBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            bufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            bufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            bufferBarrier.buffer = buffer;
            bufferBarrier.offset = 0;
            bufferBarrier.size = VK_WHOLE_SIZE;

            vkCmdPipelineBarrier(
                commandBuffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT,          // src stage
                VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,    // dst stage
                0,
                0, nullptr,
                1, &bufferBarrier,
                0, nullptr
            );
        }

        #pragma region ImGui
        void initImGui()
        {
            int pool_object_size = 100;
            VkDescriptorPoolSize pool_sizes[] = { { VK_DESCRIPTOR_TYPE_SAMPLER, pool_object_size },
                { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, pool_object_size },
                { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, pool_object_size },
                { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, pool_object_size },
                { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, pool_object_size },
                { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, pool_object_size },
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, pool_object_size },
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, pool_object_size },
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, pool_object_size },
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, pool_object_size },
                { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, pool_object_size } };

            VkDescriptorPoolCreateInfo pool_info = {};
            pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
            pool_info.maxSets = pool_object_size;
            pool_info.poolSizeCount = (uint32_t)std::size(pool_sizes);
            pool_info.pPoolSizes = pool_sizes;

            VkDescriptorPool imguiPool;
            vkCreateDescriptorPool(device, &pool_info, nullptr, &imguiPool);

            ImGui::CreateContext();

            ImGui_ImplGlfw_InitForVulkan(this->window->window, true);
            ImGui_ImplVulkan_InitInfo initInfo = {};

            initInfo.Instance = vkInstance;
            initInfo.PhysicalDevice = physicalDevice;
            initInfo.Device = device;
            initInfo.QueueFamily = *std::move(findQueueFamilies(physicalDevice).graphicsFamily);
            initInfo.Queue = graphicsQueue;
            //initInfo.PipelineCache = g_PipelineCache;
            initInfo.DescriptorPool = imguiPool;
            initInfo.RenderPass = renderPass;
            initInfo.Subpass = 0;
            initInfo.MinImageCount = 3;
            initInfo.ImageCount = 3;
            initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
            initInfo.Allocator = nullptr;
            initInfo.CheckVkResultFn = check_vk_result;

            ImGui_ImplVulkan_Init(&initInfo);
            ImGui_ImplVulkan_CreateFontsTexture();

            wd->Surface = surface;

            // Select Surface Format
            const VkFormat requestSurfaceImageFormat[] = { VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM };
            const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
            wd->SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(physicalDevice, wd->Surface, requestSurfaceImageFormat, (size_t)IM_ARRAYSIZE(requestSurfaceImageFormat), requestSurfaceColorSpace);

            VkPresentModeKHR present_modes[] = { VK_PRESENT_MODE_FIFO_KHR };
            wd->PresentMode = ImGui_ImplVulkanH_SelectPresentMode(physicalDevice, wd->Surface, &present_modes[0], IM_ARRAYSIZE(present_modes));
            //ImGui_ImplVulkanH_CreateOrResizeWindow(instance, physicalDevice, device, wd, *std::move(findQueueFamilies(physicalDevice).graphicsFamily), nullptr, WINDOW_WIDTH, WINDOW_HEIGHT, 3);
        }

        void registerTexturesToImGui()
        {
            // Register all textures to ImGui
            for (auto texture : gameManager.textures)
            {
                //ImGui_ImplVulkan_AddTexture(texture.second.textureSampler, texture.second.textureImageView, texture.second.descriptorSet);
            }
        }

        #pragma endregion

        #pragma region Compute raytracing 
		void prepareForComputeRaytracing()
		{
			// Update the ray tracing descriptor set
            descriptorManager.updateRayTracingDescriptorSet();

            // Begin command buffer recording
            {
                VkCommandBufferBeginInfo beginInfo{};
                beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

                if (vkBeginCommandBuffer(rayTracingCommandBuffers[currentFrame], &beginInfo) != VK_SUCCESS)
                {
                    throw std::runtime_error("failed to begin recording command buffer!");
                }
            }

            vkCmdResetQueryPool(rayTracingCommandBuffers[currentFrame], timestampQueryPool, 4 * currentFrame + 0, 2);

            sendBufferSizesToCompute();

            // Mandatory image barrier
            imageBarrierToGeneral(rayTracingCommandBuffers[currentFrame]);

			// Bind the ray tracing pipeline and descriptor set
            vkCmdBindPipeline(rayTracingCommandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_COMPUTE, rayTracingPipeline);
            vkCmdBindDescriptorSets(
                rayTracingCommandBuffers[currentFrame],
                VK_PIPELINE_BIND_POINT_COMPUTE,
                rayTracingPipelineLayout,
                0, // firstSet
                1, &rayTracingDescriptorSet[currentFrame],
                0, nullptr
            );
		}

        #pragma region Data transfer to compute
        void sendDataToCompute()
        {
            if (firstFrame)
            {
				// Create non dynamic buffers
                sendBvhDataToCompute();
                sendTriangleDataToCompute();

                firstFrame = false;
            }

            sendTextureDataToCompute();
            sendBvhInstancesDataToCompute();
            sendLightDataToCompute();
            sendCameraDataToCompute();
        }

        void sendBufferSizesToCompute()
        {
            // Send the sizes of the buffers to the compute shader via push constants
            GameScene scene = gameManager.gameScenes[gameManager.currentScene];

            PushConstants pc{};
            //pc.
            size_t bvhNodeSize = bvhNodes.size();
            size_t triangleSize = bvhTriangles.size();
            size_t instanceSize = bvhInstances.size();
            size_t lightInstanceSize = scene.lightSources.size();

            int data[computePushConstantCountInteger] = { bvhNodeSize , triangleSize, instanceSize, lightInstanceSize };

            vkCmdPushConstants(rayTracingCommandBuffers[currentFrame], rayTracingPipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, integerByteSize * computePushConstantCountInteger, data);
        }

        void sendTextureDataToCompute()
        {
            // TODO
        }

        void sendBvhDataToCompute()
        {
            size_t instanceCount = bvhNodes.size();
            int actualBufferSize = bvhNodes.size() * sizeof(BVHNode);
            int fullBufferSize = sizeof(uint32_t) + bvhNodes.size() * sizeof(BVHNode);

            {
                void* data;
                vkMapMemory(device, bvhBufferMemory, 0, fullBufferSize, 0, &data);
                //memcpy(data, &instanceCount, sizeof(uint32_t));
                //memcpy(static_cast<char*>(data) + sizeof(uint32_t), bvhNodes.data(), actualBufferSize);
                memcpy(data, bvhNodes.data(), actualBufferSize);
                vkUnmapMemory(device, bvhBufferMemory);
            }

            /*
            {
                // Map and copy data to staging buffer
                void* data;
                vkMapMemory(device, bvhStagingBufferMemory, 0, actualBufferSize, 0, &data);
                memcpy(data, bvhNodes.data(), actualBufferSize);
                vkUnmapMemory(device, bvhStagingBufferMemory);

                // Copy from staging buffer to device buffer
                VkCommandBuffer commandBuffer = beginSingleTimeCommands(device, commandPool);
                VkBufferCopy copyRegion{};
                copyRegion.srcOffset = 0;
                copyRegion.dstOffset = 0;
                copyRegion.size = actualBufferSize;
                vkCmdCopyBuffer(commandBuffer, bvhStagingBuffer, bvhBuffer, 1, &copyRegion);
                endSingleTimeCommands(device, commandPool, graphicsQueue, commandBuffer);
            }
            */
        }

        void sendTriangleDataToCompute()
        {
            size_t instanceCount = bvhTriangles.size();
			int actualBufferSize = bvhTriangles.size() * sizeof(Triangle);
            int fullBufferSize = sizeof(uint32_t) + bvhTriangles.size() * sizeof(Triangle);

            {
                void* data;
                vkMapMemory(device, triangleBufferMemory, 0, fullBufferSize, 0, &data);
                //memcpy(data, &instanceCount, sizeof(uint32_t));
                //memcpy(static_cast<char*>(data) + sizeof(uint32_t), bvhTriangles.data(), actualBufferSize);
                memcpy(data, bvhTriangles.data(), actualBufferSize);
                vkUnmapMemory(device, triangleBufferMemory);
			}

            /*
            {
                // Copy from staging buffer to device buffer
                VkCommandBuffer commandBuffer = beginSingleTimeCommands(device, commandPool);
                VkBufferCopy copyRegion{};
                copyRegion.srcOffset = 0;
                copyRegion.dstOffset = 0;
                copyRegion.size = actualBufferSize;
                vkCmdCopyBuffer(commandBuffer, triangleStagingBuffer, triangleBuffer, 1, &copyRegion);
                endSingleTimeCommands(device, commandPool, graphicsQueue, commandBuffer);
            }
            */
        }

        void sendBvhInstancesDataToCompute()
        {
			// Compile the data for the BVH instances
            GameScene& scene = gameManager.gameScenes[gameManager.currentScene];

            for (GameObject& gameObject : scene.gameObjects)
            {
                gameObject.updateTransform();

                if (gameObject.bvhInstanceIndex >= 0)
                {
                    bvhInstances[gameObject.bvhInstanceIndex].modelMatrix = gameObject.calculateModel();
                    continue;
                }

                GameModel* model = &gameManager.models[gameObject.model];
                gameObject.bvhInstanceIndex = bvhInstances.size();

                BVHInstance bvhInstance{};
                bvhInstance.bvhRootNodeIndex = model->bvhRootNodeIndex;
                bvhInstance.triangleOffset = model->bvhTriangleIndex;
                bvhInstance.triangleCount = model->triangles.size();
                bvhInstance.modelMatrix = gameObject.calculateModel();
                bvhInstances.push_back(bvhInstance);
            }

            size_t instanceCount = bvhInstances.size();
            int actualBufferSize = bvhInstances.size() * sizeof(BVHInstance);
			uint32_t fullBufferSize = sizeof(uint32_t) + bvhInstances.size() * sizeof(BVHInstance);

            /*
            while (actualBufferSize < bufferDefaultValue / 1.5)
            {
                BVHInstance bvhInstance{};
                bvhInstance.bvhRootNodeIndex = -1;
                bvhInstance.triangleOffset = 0;
                bvhInstance.triangleCount = 0;
                bvhInstance.modelMatrix = glm::mat4(1);
                bvhInstances.push_back(bvhInstance);

                actualBufferSize = bvhInstances.size() * sizeof(BVHInstance);
            }
            */

            if (fullBufferSize == 0)
            {
                std::cout << "WARNING: actualBufferSize is zero!" << std::endl;
                return;
            }
            if (fullBufferSize > bufferDefaultValue)
            {
                std::cout << "WARNING: actualBufferSize is bigger than allocated memory!" << std::endl;
                return;
            }

            {
                void* data;
                vkMapMemory(device, instanceBufferMemory, 0, fullBufferSize, 0, &data);
                //memcpy(data, &instanceCount, sizeof(uint32_t));
                //memcpy(static_cast<char*>(data) + sizeof(uint32_t), bvhInstances.data(), actualBufferSize);
                memcpy(data, bvhInstances.data(), actualBufferSize);
                vkUnmapMemory(device, instanceBufferMemory);
            }

            /*
            {
                // Map and copy data to staging buffer
                void* data;
                vkMapMemory(device, instanceStagingBufferMemory, 0, actualBufferSize, 0, &data);
                memcpy(data, bvhInstances.data(), actualBufferSize);
                vkUnmapMemory(device, instanceStagingBufferMemory);

                // Copy from staging buffer to device buffer
                VkCommandBuffer commandBuffer = beginSingleTimeCommands(device, commandPool);
                VkBufferCopy copyRegion{};
                copyRegion.srcOffset = 0;
                copyRegion.dstOffset = 0;
                copyRegion.size = actualBufferSize;
                vkCmdCopyBuffer(commandBuffer, instanceStagingBuffer, instanceBuffer, 1, &copyRegion);
                bufferBarrier(commandBuffer, instanceBuffer);
                endSingleTimeCommands(device, commandPool, graphicsQueue, commandBuffer);
            }
            */
        }

        void sendLightDataToCompute()
        {
            GameScene scene = gameManager.gameScenes[gameManager.currentScene];
            int lightCount = scene.lightSources.size();

            // Prevent invalid copy
            if (lightCount == 0)
            {
                return;
            }

			std::vector<LightInstance> lightArray(lightCount);
            size_t lightIndex = 0;

            // Then copy light structs
            for (auto& gameObject : scene.gameObjects)
            {
                if (gameObject.isLight)
                {
                    lightArray[lightIndex].position = gameObject.getPosition();
                    lightArray[lightIndex].color = gameObject.color;
                    lightArray[lightIndex].intensity = gameObject.intensity;
                    ++lightIndex;
                }
            }

            int actualBufferSize = lightCount * sizeof(LightInstance);

            // Map and copy data to staging buffer
            void* data;
            vkMapMemory(device, lightStagingBufferMemory, 0, actualBufferSize, 0, &data);
            memcpy(data, lightArray.data(), actualBufferSize);
            vkUnmapMemory(device, lightStagingBufferMemory);

            // Copy from staging buffer to device buffer
            VkCommandBuffer commandBuffer = beginSingleTimeCommands();
            VkBufferCopy copyRegion{};
            copyRegion.srcOffset = 0;
            copyRegion.dstOffset = 0;
            copyRegion.size = actualBufferSize;
            vkCmdCopyBuffer(commandBuffer, lightStagingBuffer, lightBuffer, 1, &copyRegion);
            endSingleTimeCommands(commandBuffer);
        }

        void sendCameraDataToCompute()
        {
			GameCamera& camera = gameManager.gameCameras[gameManager.currentCamera];
            CameraUBO ubo = camera.computeCameraData(window->WINDOW_WIDTH, window->WINDOW_HEIGHT);

            void* data;
            vkMapMemory(device, cameraBufferMemory, 0, sizeof(CameraUBO), 0, &data);
            memcpy(data, &ubo, sizeof(CameraUBO));
            vkUnmapMemory(device, cameraBufferMemory);
        }
        #pragma endregion

        void renderComputeRaytracedScene(double deltaTime)
        {
            vkCmdWriteTimestamp(rayTracingCommandBuffers[currentFrame], VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, timestampQueryPool, 4 * currentFrame + 0);

            int localSizeX = 16;
			int localSizeY = 16;
			int width = swapChainExtent.width;
			int height = swapChainExtent.height;

            vkCmdDispatch(
                rayTracingCommandBuffers[currentFrame],
                (width + localSizeX - 1) / localSizeX,
                (height + localSizeY - 1) / localSizeY,
                1
            );

            vkCmdWriteTimestamp(rayTracingCommandBuffers[currentFrame], VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, timestampQueryPool, 4 * currentFrame + 1);
        }

        void finishComputeRaytracing()
        {
            if (vkEndCommandBuffer(rayTracingCommandBuffers[currentFrame]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to record command buffer!");
            }
        };

        #pragma endregion

        #pragma region Rasterization
        void prepareForRasterization()
        {
            /// Begin command buffer recording
            {
                VkCommandBufferBeginInfo beginInfo{};
                beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

                if (vkBeginCommandBuffer(commandBuffers[currentFrame], &beginInfo) != VK_SUCCESS) {
                    throw std::runtime_error("failed to begin recording command buffer!");
                }
            }

            vkCmdResetQueryPool(commandBuffers[currentFrame], timestampQueryPool, 4 * currentFrame + 2, 2);

            vkCmdWriteTimestamp(commandBuffers[currentFrame], VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, timestampQueryPool, 4 * currentFrame + 2);

            imageBarrierToGeneral(commandBuffers[currentFrame]);

            /// Begin render pass
            {
                VkRenderPassBeginInfo renderPassInfo{};
                renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                renderPassInfo.renderPass = renderPass;
                renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
                renderPassInfo.renderArea.offset = { 0, 0 };
                renderPassInfo.renderArea.extent = swapChainExtent;

                std::array<VkClearValue, 2> clearValues{};
                clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
                clearValues[1].depthStencil = { 1.0f, 0 };

                renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
                renderPassInfo.pClearValues = clearValues.data();

                vkCmdBeginRenderPass(commandBuffers[currentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

                vkCmdBindPipeline(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

                VkViewport viewport{};
                viewport.x = 0.0f;
                viewport.y = 0.0f;
                viewport.width = (float)swapChainExtent.width;
                viewport.height = (float)swapChainExtent.height;
                viewport.minDepth = 0.0f;
                viewport.maxDepth = 1.0f;
                vkCmdSetViewport(commandBuffers[currentFrame], 0, 1, &viewport);

                VkRect2D scissor{};
                scissor.offset = { 0, 0 };
                scissor.extent = swapChainExtent;
                vkCmdSetScissor(commandBuffers[currentFrame], 0, 1, &scissor);
            }
        };

        void renderRasterizedScene(double deltaTime)
        {
            // Render current scene objects (non UI)
            GameScene& scene = gameManager.gameScenes[gameManager.currentScene];

            GameCamera cam = gameManager.getCurrentCamera();
            glm::mat4 viewProj = cam.calculateProjectionMatrix() * cam.calculateViewMatrix();
            Frustum frustum = extractFrustumPlanes(viewProj);

            // Update light uniforms
            uniformManager.updateLightUniformBuffer(scene.lightSources);

            // Cull not visible objects with multithreading
            const unsigned int totalObjects = scene.gameObjects.size();
            const int batchSize = (totalObjects + numThreads - 1) / numThreads;
            std::vector<std::future<void>> futures;
			const int num_parallel_works = std::min(numThreads, totalObjects);

            for (unsigned int t = 0; t < numThreads; ++t)
            {
                int unsigned start = t * batchSize;
                int end = std::min(start + batchSize, totalObjects);

                futures.emplace_back(std::async(std::launch::async, [start, end, &scene, &frustum]()
                    {
                        for (int i = start; i < end; ++i)
                        {
                            GameObject& obj = scene.gameObjects[i];
                            obj.isVisible = isAABBVisible(obj.minBound, obj.maxBound, obj.worldMinBound, obj.worldMaxBound, obj.calculateModel(), frustum);
                        }
                    }));
            }

            for (auto& f : futures) f.get(); // Wait for all

            // Record draw calls
            uint32_t gameObjectIndex = 0;
            for (auto gameObject : scene.gameObjects)
            {
                if (!gameObject.isVisible && !gameObject.isTerrain)
                {
                    continue;
                }
                // Update object specific uniforms
                uniformManager.updateObjectUniformBuffer(gameObject, gameObjectIndex);
                // Update descriptor sets (shader inputs)
                commandManager.recordCommandBuffer(gameObjectIndex++, gameObject);
            }
        }

        void renderUI()
        {
            // Start UI render by initialising new frame
            {
                ImGui_ImplVulkan_NewFrame();
                ImGui_ImplGlfw_NewFrame();
                ImGui::NewFrame();
            }

            // Declare all UI elements to be rendered
            {
                //scene.canvas.render(window->WINDOW_WIDTH, window->WINDOW_HEIGHT);
				gameManager.canvases[gameManager.currentCanvas].render(window->WINDOW_WIDTH, window->WINDOW_HEIGHT);

                //ImGui::ShowDemoWindow();
                // render a simple window

                /*
                ImGui::Begin("Hello, world!");
                ImGui::Text("This is some useful text.");
                ImTextureID texId = reinterpret_cast<ImTextureID>(gameManager.textures["default"].id);
                ImGui::Image(texId, ImVec2(128, 128));
                ImGui::End();
                */
            }

            // Render call
            {
                ImGui::Render();
                ImDrawData* draw_data = ImGui::GetDrawData();
                if (draw_data) {
                    ImGui_ImplVulkan_RenderDrawData(draw_data, commandBuffers[currentFrame]);
                }
            }
        }

		void finishRasterization()
		{
            vkCmdWriteTimestamp(commandBuffers[currentFrame], VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, timestampQueryPool, 4 * currentFrame + 3);
		}

        #pragma endregion

        #pragma region Compositing
        void compositeRaytracingAndRasterization()
        {
            descriptorManager.updateCompositingDescriptorSet();

            // Begin command buffer recording
            {
                VkCommandBufferBeginInfo beginInfo{};
                beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

                if (vkBeginCommandBuffer(compositingCommandBuffers[currentFrame], &beginInfo) != VK_SUCCESS)
                {
                    throw std::runtime_error("failed to begin recording command buffer!");
                }
            }

            transitionImageLayout(compositingCommandBuffers[currentFrame], swapChainImages[currentFrame], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

            //descriptorManager.updateCompositingDescriptorSet();

            // Bind the compute pipeline and descriptor set
            vkCmdBindPipeline(compositingCommandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_COMPUTE, compositingPipeline);
            vkCmdBindDescriptorSets(
                compositingCommandBuffers[currentFrame],
                VK_PIPELINE_BIND_POINT_COMPUTE,
                compositingPipelineLayout,
                0, // firstSet
                1, &compositingDescriptorSet[currentFrame],
                0, nullptr
            );

            //imageBarrier(compositingCommandBuffers[currentFrame]);

            compositeFrames();
            finishCompositing();
        }

        void compositeFrames()
        {
            int localSizeX = 16;
            int localSizeY = 16;
            int width = swapChainExtent.width;
            int height = swapChainExtent.height;

            vkCmdDispatch(
                compositingCommandBuffers[currentFrame],
                (width + localSizeX - 1) / localSizeX,
                (height + localSizeY - 1) / localSizeY,
                1
            );
        }

        void finishCompositing()
        {
            transitionImageLayout(compositingCommandBuffers[currentFrame], swapChainImages[currentFrame], VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

            if (vkEndCommandBuffer(compositingCommandBuffers[currentFrame]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to record command buffer!");
            }
        };
        #pragma endregion

        void finishFrameRendering()
        {
            // End render pass and command buffer
            {
                vkCmdEndRenderPass(commandBuffers[currentFrame]);

                if (vkEndCommandBuffer(commandBuffers[currentFrame]) != VK_SUCCESS)
                {
                    throw std::runtime_error("failed to record command buffer!");
                }
            }

            /// Submit commands
            VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
            {
                VkSubmitInfo submitInfo{};
                submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

                VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
                VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
                submitInfo.waitSemaphoreCount = 1;
                submitInfo.pWaitSemaphores = waitSemaphores;
                submitInfo.pWaitDstStageMask = waitStages;

                VkCommandBuffer cmdBuffers[] = { commandBuffers[currentFrame], rayTracingCommandBuffers[currentFrame] };

                submitInfo.commandBufferCount = 2;
                submitInfo.pCommandBuffers = cmdBuffers;

                //VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
                submitInfo.signalSemaphoreCount = 1;
                submitInfo.pSignalSemaphores = signalSemaphores;

                if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
                {
                    throw std::runtime_error("failed to submit draw command buffer!");
                }
            }

            // Read timestamps
            {
                vkGetQueryPoolResults(device, timestampQueryPool, 4 * currentFrame, 4, sizeof(timestamps), timestamps, sizeof(uint64_t), VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);
            }

            /// Present the frame
            {
                VkPresentInfoKHR presentInfo{};
                presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

                presentInfo.waitSemaphoreCount = 1;
                presentInfo.pWaitSemaphores = signalSemaphores;

                VkSwapchainKHR swapChains[] = { swapChain };
                presentInfo.swapchainCount = 1;
                presentInfo.pSwapchains = swapChains;

                presentInfo.pImageIndices = &imageIndex;

                VkResult result = vkQueuePresentKHR(presentQueue, &presentInfo);

                if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window->framebufferResized)
                {
                    window->framebufferResized = false;
                    swapChainManager.recreateSwapChain(instance.window);
                }
                else if (result != VK_SUCCESS)
                {
                    throw std::runtime_error("failed to present swap chain image!");
                }
            }

            /// Advance to the next frame
            {
                currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
            }
        }

	public:
        VulkanRenderer(Window* window)
            :
            instance(),
            deviceManager()
            /*
            swapChainManager(window->window),
            pipeline(),
            commandManager(),
            textureManager(),
            modelManager(),
            uniformManager(),
            descriptorManager(),
            syncManager()
            */
        {
            window->initWindowAndCallbacks();
            this->instance.createWindowSurface(window->window);
            this->deviceManager.init();
            this->swapChainManager.init(window->window);
            this->pipeline.init();
            this->commandManager.init();
            this->textureManager.init();
            this->modelManager.init();
            this->uniformManager.init();
            this->descriptorManager.init();
            this->syncManager.init();

			this->window = window;
            this->initImGui();
            this->textureManager.createVulkanTextures("Resources/Textures/");
            this->modelManager.createVulkanModels("Resources/Models/");
            this->loadDefaultScene();
			this->commandManager.initCommandBuffers();
			this->descriptorManager.preallocateDescriptorSets();
			this->swapChainManager.recreateSwapChain(window->window);

            auto queueFamilies = findQueueFamilies(physicalDevice);
            assert(queueFamilies.graphicsFamily.has_value());
            uint32_t graphicsQueueIndex = queueFamilies.graphicsFamily.value();

            VkQueryPoolCreateInfo queryPoolInfo{};
            queryPoolInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
            queryPoolInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
            queryPoolInfo.queryCount = 4 * MAX_FRAMES_IN_FLIGHT;

            vkCreateQueryPool(device, &queryPoolInfo, nullptr, &timestampQueryPool);

            /*
            ImGui_ImplVulkan_SetMinImageCount(3);
            ImGui_ImplVulkanH_DestroyWindow(vkInstance, device, &g_MainWindowData, nullptr);
            ImGui_ImplVulkanH_CreateOrResizeWindow(
                vkInstance,
                physicalDevice,
                device,
                &g_MainWindowData,
                graphicsQueueIndex,
                nullptr,
                window->WINDOW_WIDTH,
                window->WINDOW_HEIGHT,
                3
            );
            */
            //ImGui_ImplVulkanH_CreateOrResizeWindow(vkInstance, physicalDevice, device, &g_MainWindowData, *std::move(findQueueFamilies(physicalDevice).graphicsFamily), nullptr, window->WINDOW_WIDTH, window->WINDOW_HEIGHT, 3);
        }

		void loadAssets(string path)
		{
			loadTextures(path);
			loadModels(path);
			loadScenes(path);
		}

		void renderFrame(double deltaTime)
		{
            this->waitForPreviousFrame();

            if (usingGpgpuRaytracing)
            {
                this->sendDataToCompute();
                this->prepareForComputeRaytracing();
                this->renderComputeRaytracedScene(deltaTime);
                this->finishComputeRaytracing();
            }
            else
            {

            }

			this->prepareForRasterization();
            this->renderRasterizedScene(deltaTime);
            this->renderUI();
            this->finishRasterization();

			this->finishFrameRendering();
		}
	};
}