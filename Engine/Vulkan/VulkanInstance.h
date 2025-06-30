#pragma once
#include "VulkanTypes.h"
#include "VulkanGlobals.h"
#include <glfw3.h>
#include <glfw3native.h>

namespace Engine
{
	class VulkanInstance
	{
	private:
        bool checkValidationLayerSupport()
        {
            uint32_t layerCount;
            vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

            std::vector<VkLayerProperties> availableLayers(layerCount);
            vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

            for (const char* layerName : validationLayers)
            {
                bool layerFound = false;

                for (const auto& layerProperties : availableLayers)
                {
                    if (strcmp(layerName, layerProperties.layerName) == 0)
                    {
                        layerFound = true;
                        break;
                    }
                }

                if (!layerFound)
                {
                    return false;
                }
            }

            return true;
        }

        std::vector<const char*> getRequiredExtensions()
        {
            uint32_t glfwExtensionCount = 0;
            const char** glfwExtensions;
            glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

            std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

            if (enableValidationLayers)
            {
                extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            }

            return extensions;
        }

        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
        {
            //std::cerr << std::endl << "Validation layer: " << std::endl << pCallbackData->pMessage << std::endl;
            //return VK_FALSE;

            if (messageSeverity > VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
            {
                std::cerr << std::endl << "Validation layer: " << std::endl << pCallbackData->pMessage << std::endl;
            }
            else
            {
				//std::cout << std::endl << "Validation layer: " << std::endl << pCallbackData->pMessage << std::endl;
            }

            return VK_FALSE;
        }

        void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
        {
            createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            //createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            createInfo.pfnUserCallback = debugCallback;
        }

        VkResult CreateDebugUtilsMessengerEXT(VkInstance vkInstance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
        {
            auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vkInstance, "vkCreateDebugUtilsMessengerEXT");

            if (func != nullptr)
            {
                return func(vkInstance, pCreateInfo, pAllocator, pDebugMessenger);
            }
            else
            {
                return VK_ERROR_EXTENSION_NOT_PRESENT;
            }
        }

	public:
        GLFWwindow* window;

		VulkanInstance()
		{
            debugVulkan && printf("\nInitializing Vulkan\n");

            auto start = std::chrono::high_resolution_clock::now();

            VkApplicationInfo appInfo{};
            appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            appInfo.pApplicationName = "VulkanGameEngine";
            appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.pEngineName = "Vulkan Prototype";
            appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.apiVersion = VK_API_VERSION_1_3;

            VkValidationFeatureEnableEXT enables[] = { VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT };
            VkValidationFeaturesEXT features = {};
            features.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
            features.enabledValidationFeatureCount = 1;
            features.pEnabledValidationFeatures = enables;

            VkInstanceCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            createInfo.pApplicationInfo = &appInfo;
            createInfo.pNext = &features;

            //auto extensions = getRequiredExtensions();
            
            const std::vector<const char*> extensions = {
                "VK_KHR_surface",
                "VK_KHR_win32_surface",
                "VK_EXT_debug_utils"
            };
            
            createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
            createInfo.ppEnabledExtensionNames = extensions.data();

            VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
            if (enableValidationLayers)
            {
                createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
                createInfo.ppEnabledLayerNames = validationLayers.data();

                populateDebugMessengerCreateInfo(debugCreateInfo);

                createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
            }
            else
            {
                createInfo.enabledLayerCount = 0;

                createInfo.pNext = nullptr;
            }

            auto end1 = std::chrono::high_resolution_clock::now();

            if (vkCreateInstance(&createInfo, nullptr, &vkInstance) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create instance!");
            }

            auto end2 = std::chrono::high_resolution_clock::now();

            if (enableValidationLayers && !checkValidationLayerSupport())
            {
                throw std::runtime_error("validation layers requested, but not available!");
            }

            if (enableValidationLayers)
            {
                VkDebugUtilsMessengerCreateInfoEXT createInfo;
                populateDebugMessengerCreateInfo(createInfo);

                if (CreateDebugUtilsMessengerEXT(vkInstance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
                {
                    throw std::runtime_error("failed to set up debug messenger!");
                }
            }

            auto end3 = std::chrono::high_resolution_clock::now();

            auto duration1 = std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start).count();
            auto duration2 = std::chrono::duration_cast<std::chrono::milliseconds>(end2 - end1).count();
            auto duration3 = std::chrono::duration_cast<std::chrono::milliseconds>(end3 - end2).count();
            debugVulkan&& printf("\nInstance:\n    populateDebugMessengerCreateInfo: %lld ms\n    vkCreateInstance: %lld ms\n    debugMessenger logic: %lld ms\n\n", duration1, duration2, duration3);
		}

        void createWindowSurface(GLFWwindow* window)
        {
            this->window = window;

            VkResult result = glfwCreateWindowSurface(vkInstance, window, nullptr, &surface);

            if (result != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create window surface!");
            }
        }
	};
};