#pragma once
#include "vulkanUtils.h"
#include "../Core/Globals.h"
#include "../Core/Game/GameObject.h"

namespace Engine {
    class VulkanUniform
    {
    private:
        uint32_t uniformCount = maxNumberOfObjects;

        void createUniformBuffers()
        {
            VkDeviceSize bufferSize = sizeof(UniformBufferObject);
            //VkDeviceSize bufferSize = gameObjects.size() * sizeof(UniformBufferObject);

            uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT * uniformCount);
            uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT * uniformCount);
            uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT * uniformCount);

            for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
            {
                for (size_t j = 0; j < uniformCount; j++)
                {
					int index = i * uniformCount + j;
                    createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[index], uniformBuffersMemory[index]);

                    vkMapMemory(device, uniformBuffersMemory[index], 0, bufferSize, 0, &uniformBuffersMapped[index]);
                }
            }
        };

    public:
        VulkanUniform() {};

        void init()
        {
            createUniformBuffers();
        };

        void updateObjectUniformBuffer(GameObject gameObject, uint32_t gameObjectIndex)
        {
            static auto startTime = std::chrono::high_resolution_clock::now();

            auto currentTime = std::chrono::high_resolution_clock::now();
            float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

            UniformBufferObject ubo{};
            if (gameObject.isStatic)
            {
                ubo.model = gameObject.calculateModel();
            }
            else if (gameObject.rigidBody != nullptr)
            {
                btTransform transform = gameObject.rigidBody->getWorldTransform();
                btVector3 position = transform.getOrigin();
                btQuaternion rotation = transform.getRotation();
                glm::vec3 glmPosition(position.x(), position.y(), position.z());
                glm::quat glmRotation(rotation.w(), rotation.x(), rotation.y(), rotation.z());

                ubo.model = glm::mat4(1.0f);
                ubo.model = glm::scale(ubo.model, glm::vec3(gameObject.scale));
                ubo.model = glm::translate(ubo.model, glmPosition);
                ubo.model *= glm::mat4_cast(glmRotation);
            }
            ubo.view = gameManager.getCurrentCamera().calculateViewMatrix();
            ubo.proj = gameManager.getCurrentCamera().calculateProjectionMatrix();
            ubo.proj[1][1] *= -1;
            ubo.cameraPos = gameManager.getCurrentCamera().position;
            ubo.cameraLookAt = gameManager.getCurrentCamera().lookAt;

			uint32_t index = gameManager.getCurrentScene().gameObjects.size() * currentFrame + gameObjectIndex;
            if (index < uniformCount)
            {
                memcpy(uniformBuffersMapped[index], &ubo, sizeof(ubo));
            }
        };

        void updateLightUniformBuffer(std::vector<GameObject> lightSources)
        {

        }
    };
};