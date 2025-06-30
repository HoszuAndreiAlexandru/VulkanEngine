#pragma once
#include <glm/glm.hpp>
#include "../../Vulkan/VulkanTypes.h"

namespace Engine
{
	class GameCamera
	{
	public:
		// View
		glm::vec3 position;
		glm::vec3 lookAt;
		glm::vec3 up;

		// Projection
		double fovy = 45.0f;
		double aspectRatio = 16.0f / 9.0f;
		double nearClip = 0.1f;
		double farClip = 10000.0f;

		float cameraSpeed = 5;
		double yaw = -90.0f;
		double pitch = 0.0f;
		double sensitivity = 0.05f;

		GameCamera()
		{
			position = glm::vec3(0.0f, 0.0f, 0.0f);
			lookAt = glm::vec3(0.0f, 0.0f, -1.0f);
			up = glm::vec3(0.0f, 1.0f, 0.0f);
		}

		glm::mat4 calculateViewMatrix() const
		{
			return glm::lookAt(position, lookAt, up);
		};

		glm::mat4 calculateProjectionMatrix() const
		{
			return glm::perspective(glm::radians(fovy), aspectRatio, nearClip, farClip);
		};

		CameraUBO computeCameraData(uint32_t screenWidth, uint32_t screenHeight, float verticalFovRadians = glm::radians(45.0f)) const
		{
			CameraUBO data;
			data.position = glm::vec4(position, 0);

			// Compute forward vector
			glm::vec3 forward = glm::normalize(lookAt - position);
			glm::vec3 right = glm::normalize(glm::cross(forward, up));
			glm::vec3 upVec = glm::normalize(glm::cross(right, forward)); // Ensure orthogonal

			data.direction = glm::vec4(forward, 0);

			float aspect = static_cast<float>(screenWidth) / static_cast<float>(screenHeight);
			double fovyRadians = glm::radians(fovy);
			float screenHeightWorld = 2.0f * tan(fovyRadians * 0.5f);
			float screenWidthWorld = screenHeightWorld * aspect;

			data.right = glm::vec4(right * screenWidthWorld * 0.5f, 0);
			data.up = glm::vec4(upVec * screenHeightWorld * 0.5f, 0);

			glm::vec3 screenCenter = position + forward;
			data.screenCenter = glm::vec4(screenCenter, 0.0f);

			data.screenDimensions = glm::vec4(swapChainExtent.width, swapChainExtent.height, 0, 0);

			data.projectionMatrix = this->calculateProjectionMatrix();
			data.viewMatrix = this->calculateViewMatrix();

			verticalFovRadians = glm::radians(fovy * 2);
			const float tanFov = tan(verticalFovRadians * 0.5f);
			const float aspect2 = float(screenWidth) / float(screenHeight);

			data.verticalFovRadians = verticalFovRadians;
			data.tanFov = tanFov;
			data.aspect = aspect2;

			return data;
		}
	};
};