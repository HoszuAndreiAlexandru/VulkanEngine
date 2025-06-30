#pragma once
#include "VulkanUtils.h"

using namespace std;

namespace Engine
{
	class VulkanDeviceManager
	{
	private:
        VkPhysicalDevice dedicatedGPU = VK_NULL_HANDLE;
        VkPhysicalDevice integratedGPU = VK_NULL_HANDLE;

        bool checkDeviceExtensionSupport(VkPhysicalDevice device)
        {
            uint32_t extensionCount;
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

            std::vector<VkExtensionProperties> availableExtensions(extensionCount);
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

            std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

            for (const auto& extension : availableExtensions)
            {
                requiredExtensions.erase(extension.extensionName);
            }

            return requiredExtensions.empty();
        }

        bool isDeviceSuitable(VkPhysicalDevice device)
        {
            QueueFamilyIndices indices = findQueueFamilies(device);

            bool extensionsSupported = checkDeviceExtensionSupport(device);

            bool swapChainAdequate = false;
            if (extensionsSupported)
            {
                SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
                swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
            }

            VkPhysicalDeviceFeatures supportedFeatures;
            vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

            return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
        }

		void pickPhysicalDevice()
		{
            auto start = std::chrono::high_resolution_clock::now();

            uint32_t deviceCount = 0;
            vkEnumeratePhysicalDevices(vkInstance, &deviceCount, nullptr);

            if (deviceCount == 0)
            {
                throw std::runtime_error("Failed to find GPUs with Vulkan support!");
            }

            std::vector<VkPhysicalDevice> devices(deviceCount);
            vkEnumeratePhysicalDevices(vkInstance, &deviceCount, devices.data());

            for (const auto& device : devices)
            {
                auto props = VkPhysicalDeviceProperties{};
                vkGetPhysicalDeviceProperties(device, &props);

                if (props.deviceType == VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
                {
                    dedicatedGPU = device;
                }
                if (props.deviceType == VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
                {
                    integratedGPU = device;
                }
            }

            auto end1 = std::chrono::high_resolution_clock::now();
            auto duration1 = std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start).count();
            debugVulkan && printf("Pick physical device: %lld ms", duration1);

            if (dedicatedGPU != VK_NULL_HANDLE)
            {
                std::cout << "\nPicked dedicated GPU.\n\n";
                physicalDevice = dedicatedGPU;
                return;
            }

            if (integratedGPU != VK_NULL_HANDLE)
            {
                std::cout << "\nPicked integrated GPU.\n\n";
                physicalDevice = integratedGPU;
                return;
            }

            if (physicalDevice == VK_NULL_HANDLE)
            {
                throw std::runtime_error("Failed to find a suitable GPU!");
            }
		}

        void createLogicalDevice()
        {
            auto start = std::chrono::high_resolution_clock::now();

            // If GPU is not suitable, exit
            if (!isDeviceSuitable(physicalDevice))
            {
                throw std::runtime_error("GPU is not suitable!");
            }

            QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

            std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
            std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

            float queuePriority = 1.0f;
            for (uint32_t queueFamily : uniqueQueueFamilies)
            {
                VkDeviceQueueCreateInfo queueCreateInfo{};
                queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queueCreateInfo.queueFamilyIndex = queueFamily;
                queueCreateInfo.queueCount = 1;
                queueCreateInfo.pQueuePriorities = &queuePriority;
                queueCreateInfos.push_back(queueCreateInfo);
            }

            VkPhysicalDeviceFeatures deviceFeatures{};
            deviceFeatures.samplerAnisotropy = VK_TRUE;
			deviceFeatures.fragmentStoresAndAtomics = VK_TRUE;

            VkDeviceCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

            createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
            createInfo.pQueueCreateInfos = queueCreateInfos.data();

            createInfo.pEnabledFeatures = &deviceFeatures;

            createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
            createInfo.ppEnabledExtensionNames = deviceExtensions.data();

            if (enableValidationLayers)
            {
                createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
                createInfo.ppEnabledLayerNames = validationLayers.data();
            }
            else
            {
                createInfo.enabledLayerCount = 0;
            }

            if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to create logical device!");
            }

            vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
            vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);

            auto end1 = std::chrono::high_resolution_clock::now();
            auto duration1 = std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start).count();
            debugVulkan && printf("    Create logical device: %lld ms\n", duration1);

            vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
        }

	public:
		VulkanDeviceManager()
		{
            pickPhysicalDevice();
		}

        void init()
        {
            this->createLogicalDevice();
        }
	};
};