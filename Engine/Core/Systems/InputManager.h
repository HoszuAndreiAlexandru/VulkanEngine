#pragma once
#include <unordered_map>
#include "../Globals.h"
#include "../../Vulkan/VulkanGlobals.h"

namespace Engine
{
	class InputManager
	{
	private:
		static bool keyboardPressIsTooFast(clock_t tStart)
		{
			return tStart / 1000 - InputManager::lastKeyPress / 1000 < 0.25f;
		}

	public:
		static std::unordered_map<int, bool> keys; // Key state storage
		static double mouseX;
		static double mouseY;
		static bool mouseButtons[3]; // Left, Right, Middle

		static bool cameraControl;
		static bool mouseCameraControlLastFrame;
		static bool mouseCameraControl;
		static float lastKeyPress;

		static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
		{
			if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE)
			{
				mouseCameraControlLastFrame = true;
				mouseCameraControl = false;
				glfwGetCursorPos(window, &mouseX, &mouseY);
			}

			if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
			{
				mouseCameraControl = true;
			}
			else
			{
				mouseCameraControl = false;
			}
		}

		static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			if (action == GLFW_PRESS)
			{
				keys[key] = true;
			}
			else if (action == GLFW_RELEASE)
			{
				keys[key] = false;
			}
		}

		static void ProcessKeyboardInput()
		{
			GameCamera& currentCamera = gameManager.gameCameras[gameManager.currentCamera];
			float cameraSpeed = currentCamera.cameraSpeed * globalDeltaTime;

			const glm::vec3 direction = currentCamera.lookAt - currentCamera.position;
			const glm::vec3 right = glm::normalize(glm::cross(direction, currentCamera.up));

			// Check for key presses
			if (keys[GLFW_KEY_W] == GLFW_PRESS)
			{
				currentCamera.lookAt += direction * cameraSpeed;
				currentCamera.position += direction * cameraSpeed;
			}
			if (keys[GLFW_KEY_S] == GLFW_PRESS)
			{
				currentCamera.lookAt -= direction * cameraSpeed;
				currentCamera.position -= direction * cameraSpeed;
			}
			if (keys[GLFW_KEY_A] == GLFW_PRESS)
			{
				currentCamera.lookAt -= right * cameraSpeed;
				currentCamera.position -= right * cameraSpeed;
			}
			if (keys[GLFW_KEY_D] == GLFW_PRESS)
			{
				currentCamera.lookAt += right * cameraSpeed;
				currentCamera.position += right * cameraSpeed;
			}
			if (keys[GLFW_KEY_E] == GLFW_PRESS)
			{
				currentCamera.lookAt -= currentCamera.up * cameraSpeed;
				currentCamera.position -= currentCamera.up * cameraSpeed;
			}
			if (keys[GLFW_KEY_Q] == GLFW_PRESS)
			{
				currentCamera.lookAt += currentCamera.up * cameraSpeed;
				currentCamera.position += currentCamera.up * cameraSpeed;
			}

			bool keyPressed = false;
			clock_t tStart = clock();

			if (keys[GLFW_KEY_C] == GLFW_PRESS && !keyboardPressIsTooFast(tStart))
			{
				keyPressed = true;
				cameraControl = !cameraControl;
				GLFWwindow* window = glfwGetCurrentContext();
				if (cameraControl == true)
				{
					glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				}
				else
				{
					glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				}
			}

			if (keyPressed)
			{
				InputManager::lastKeyPress = tStart;
			}
		}

		static void CursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
		{
			if (!cameraControl && !mouseCameraControl)
			{
				return;
			}

			GameCamera& currentCamera = gameManager.gameCameras[gameManager.currentCamera];

			// Calculate the offset from the last mouse position
			double xoffset = (xpos - swapChainExtent.width / 2) * currentCamera.sensitivity;
			double yoffset = (swapChainExtent.height / 2 - ypos) * currentCamera.sensitivity;

			if (mouseCameraControlLastFrame)
			{
				mouseX = xpos;
				mouseY = ypos;
				mouseCameraControlLastFrame = false;
			}

			if (mouseCameraControl)
			{
				xoffset = (xpos - mouseX) * currentCamera.sensitivity;
				yoffset = (mouseY - ypos) * currentCamera.sensitivity;
			}
			else
			{
				glfwSetCursorPos(window, swapChainExtent.width / 2, swapChainExtent.height / 2);
			}

			currentCamera.yaw -= xoffset; /// horizontal
			currentCamera.pitch += yoffset; /// vertical

			if (currentCamera.pitch > 89.9f) currentCamera.pitch = 89.9f;
			if (currentCamera.pitch < -89.9f) currentCamera.pitch = -89.9f;

			// Update camera front vector based on yaw and pitch

			glm::vec3 front = glm::vec3(
				cos(glm::radians(currentCamera.pitch)) * sin(glm::radians(currentCamera.yaw)),
				sin(glm::radians(currentCamera.pitch)),
				cos(glm::radians(currentCamera.pitch)) * cos(glm::radians(currentCamera.yaw))
			);

			currentCamera.lookAt = currentCamera.position + front;

			mouseX = xpos;
			mouseY = ypos;
		}

		// Helper functions
		static bool IsKeyPressed(int key) { return keys[key]; }
		static bool IsMouseButtonPressed(int button) { return mouseButtons[button]; }
	};

	// Initialize static variables
	std::unordered_map<int, bool> InputManager::keys;
	double InputManager::mouseX = 0;
	double InputManager::mouseY = 0;
	bool InputManager::mouseButtons[3] = { false, false, false };
	bool InputManager::cameraControl = false;
	bool InputManager::mouseCameraControlLastFrame = false;
	bool InputManager::mouseCameraControl = false;
	float InputManager::lastKeyPress = 0;
};