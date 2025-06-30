#pragma once
#include "InputManager.h"
#include "../../Vulkan/VulkanUtils.h"

namespace Engine
{
	class Window
	{
	public:
		GLFWwindow* window;
		const char* WINDOW_TITLE = "Game";
		uint32_t WINDOW_WIDTH = 1280;
		uint32_t WINDOW_HEIGHT = 720;
		bool framebufferResized = false;

		Window()
		{
			glfwInit();
			glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		};

		void initWindowAndCallbacks()
		{
			window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, nullptr, nullptr);
			glfwSetWindowUserPointer(window, this);

			/// Set callback functions
			glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
			glfwSetKeyCallback(window, InputManager::KeyCallback);
			glfwSetMouseButtonCallback(window, InputManager::MouseButtonCallback);
			glfwSetCursorPosCallback(window, InputManager::CursorPositionCallback);

			/// Capture mouse position to the center
			glfwSetCursorPos(window, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
			InputManager::mouseX = WINDOW_WIDTH / 2;
			InputManager::mouseY = WINDOW_HEIGHT / 2;
		}

		bool shouldClose()
		{
			return glfwWindowShouldClose(window);
		}

		void pollEvents()
		{
			glfwPollEvents();
		}

		void renameWindow(char* newTitle)
		{
			glfwSetWindowTitle(window, newTitle);
		}

		void static framebufferResizeCallback(GLFWwindow* window, int width, int height)
		{
			Window* windowInstance = static_cast<Window*>(glfwGetWindowUserPointer(window));
			if (windowInstance)
			{
				windowInstance->framebufferResized = true;
			}

			windowInstance->WINDOW_HEIGHT = height;
			windowInstance->WINDOW_WIDTH = width;

			ImGui_ImplVulkan_SetMinImageCount(3);
			ImGui_ImplVulkanH_CreateOrResizeWindow(vkInstance, physicalDevice, device, &g_MainWindowData, *std::move(findQueueFamilies(physicalDevice).graphicsFamily), nullptr, windowInstance->WINDOW_WIDTH, windowInstance->WINDOW_HEIGHT, 3);
		}
	};
};