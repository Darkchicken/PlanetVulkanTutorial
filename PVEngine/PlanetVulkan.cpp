#include "PlanetVulkan.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <cstring>
#include <vector>
#include <map>
#include <algorithm>

namespace PVEngine
{

	PlanetVulkan::PlanetVulkan()
	{
	}


	PlanetVulkan::~PlanetVulkan()
	{
	}

	void PlanetVulkan::InitVulkan()
	{
		windowObj.Create();
		CreateInstance();
		SetupDebugCallback();
		CreateSurface();
		GetPhysicalDevices();
		CreateLogicalDevice();
	}

	void PlanetVulkan::CreateInstance()
	{
		
		//check for validation layer availability
		if (enableValidationLayers && !CheckValidationLayerSupport())
		{
			throw std::runtime_error("Validation layers requested but not available");
		}
		else
		{
			std::cout << "Requested layers available" << std::endl;
		}
	
		
		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pNext = nullptr;
		appInfo.pApplicationName = "Planet Vulkan";
		appInfo.applicationVersion = VK_MAKE_VERSION(1,0,0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.pApplicationInfo = &appInfo;

		if (enableValidationLayers)
		{
			createInfo.enabledLayerCount = validationLayers.size();
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else
		{
			createInfo.enabledLayerCount = 0;
			createInfo.ppEnabledLayerNames = nullptr;
		}
	
		auto extensions = GetRequiredExtensions();
		createInfo.enabledExtensionCount = extensions.size();
		createInfo.ppEnabledExtensionNames = extensions.data();

		if (vkCreateInstance(&createInfo, nullptr, instance.replace()) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create instance!");
		}
		else
		{
			std::cout << "Vulkan instance create successfully" << std::endl;
		}
	}

	bool PlanetVulkan::CheckValidationLayerSupport()
	{
		std::cout << "Checking Support..." << std::endl;
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount,nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName : validationLayers)
		{
			bool layerFound = false;
			for (const auto &layerProperties : availableLayers)
			{
				if (strcmp(layerName, layerProperties.layerName) == 0)
				{
					layerFound = true;
					break;
				}
			}
			if(!layerFound)
			{
				return false;
			}
		}
		return true;
	}

	std::vector<const char*> PlanetVulkan::GetRequiredExtensions()
	{
		std::vector<const char*> extensions;

		unsigned int glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		for (unsigned int i = 0; i < glfwExtensionCount; i++)
		{
			extensions.push_back(glfwExtensions[i]);
		}

		if (enableValidationLayers)
		{
			extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
		}

		return extensions;
	}

	void PlanetVulkan::SetupDebugCallback()
	{
		if(!enableValidationLayers)
		{
			return;
		}

		VkDebugReportCallbackCreateInfoEXT createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
		createInfo.pfnCallback = debugCallback;

		if (CreateDebugReportCallbackEXT(instance, &createInfo, nullptr, callback.replace()) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to set up debug callback");
		}
		else
		{
			std::cout << "Debug Callback setup successful" << std::endl;
		}
	}

	void PlanetVulkan::CreateSurface()
	{
		if (glfwCreateWindowSurface(instance, windowObj.window , nullptr, surface.replace()) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create window surface");
		}
		else
		{
			std::cout << "Window surface created successfully" << std::endl;
		}
	}

	void PlanetVulkan::GetPhysicalDevices()
	{
		uint32_t physicalDeviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);
		if (physicalDeviceCount == 0)
		{
			throw std::runtime_error("No devices found with Vulkan supprt");
		}

		std::vector<VkPhysicalDevice> foundPhysicalDevices(physicalDeviceCount);
		vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, foundPhysicalDevices.data());

		std::multimap<int, VkPhysicalDevice> rankedDevices;

		for (const auto& currentDevice : foundPhysicalDevices)
		{
			int score = RateDeviceSuitability(currentDevice);
			rankedDevices.insert(std::make_pair(score, currentDevice));
		}
		
		if (rankedDevices.rbegin()->first > 0)
		{
			physicalDevice = rankedDevices.rbegin()->second;		
			std::cout << "Physical device found" << std::endl;	
		}
		else
		{
			throw std::runtime_error("No physical devices meet necessary criteria");
		}
	}

	int PlanetVulkan::RateDeviceSuitability(VkPhysicalDevice deviceToRate)
	{
		int score = 0;

		QueueFamilyIndices indices = FindQueueFamilies(deviceToRate);
		bool extensionsSupported = CheckDeviceExtensionSupport(deviceToRate);
		if (!indices.isComplete() || !extensionsSupported)
		{
			return 0;
		}
		
		bool swapChainAdequate = false;
		SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(deviceToRate);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		if (!swapChainAdequate)
		{
			return 0;
		}
		

		VkPhysicalDeviceFeatures deviceFeatures;
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(deviceToRate,&deviceProperties);
		vkGetPhysicalDeviceFeatures(deviceToRate,&deviceFeatures);

		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			score += 1000;
		}

		score += deviceProperties.limits.maxImageDimension2D;

		if (!deviceFeatures.geometryShader)
		{
			return 0;
		}

		return score;
	}

	void PlanetVulkan::CreateLogicalDevice()
	{
		QueueFamilyIndices indices = FindQueueFamilies(physicalDevice);

		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.pNext = nullptr;
		queueCreateInfo.flags = 0;
		queueCreateInfo.queueFamilyIndex = indices.displayFamily;
		queueCreateInfo.queueCount = 1;
		const float queuePriority = 1.0f;
		queueCreateInfo.pQueuePriorities = &queuePriority;

		VkPhysicalDeviceFeatures deviceFeatures = {};

		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.queueCreateInfoCount = 1;
		createInfo.pQueueCreateInfos = &queueCreateInfo;
		if (enableValidationLayers)
		{
			createInfo.enabledLayerCount = validationLayers.size();
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else
		{
			createInfo.enabledLayerCount = 0;
			createInfo.ppEnabledLayerNames = nullptr;
		}
		createInfo.enabledExtensionCount = deviceExtensions.size();
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();
		createInfo.pEnabledFeatures = &deviceFeatures;

		if (vkCreateDevice(physicalDevice, &createInfo , nullptr, logicalDevice.replace()) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create logical device");
		}
		else
		{
			std::cout << "Logical device created successfully" << std::endl;
		}

		vkGetDeviceQueue(logicalDevice, indices.displayFamily, 0, &displayQueue);
	}

	QueueFamilyIndices PlanetVulkan::FindQueueFamilies(VkPhysicalDevice device)
	{
		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto &queueFamily : queueFamilies)
		{
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

			if (queueFamily.queueCount > 0 && queueFamily.queueFlags && VK_QUEUE_GRAPHICS_BIT && presentSupport)
			{
				indices.displayFamily = i;
			}

			if (indices.isComplete())
			{
				break;
			}
			i++;
		}
		return indices;
	}
	bool PlanetVulkan::CheckDeviceExtensionSupport(VkPhysicalDevice device)
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		for (const char* currentExtension : deviceExtensions)
		{
			bool extensionFound = false;
			for (const auto& extention : availableExtensions)
			{
				if (strcmp(currentExtension, extention.extensionName) == 0)
				{
					extensionFound = true;
					break;
				}
			}
			if (!extensionFound)
			{
				return false;
			}
		}
		return true;

	}

	SwapChainSupportDetails PlanetVulkan::QuerySwapChainSupport(VkPhysicalDevice device)
	{
		SwapChainSupportDetails details;

		// capabilities
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

		//formats
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

		if (formatCount != 0)
		{
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
		}

		//presentModes
		uint32_t presentModesCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModesCount, nullptr);
		if (presentModesCount != 0)
		{
			details.presentModes.resize(presentModesCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModesCount, details.presentModes.data());
		}

		return details;
	}

	VkSurfaceFormatKHR PlanetVulkan::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		// if surface has no preferred format
		if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED)
		{
			return{VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
		}

		for (const auto& currentFormat : availableFormats)
		{
			if (currentFormat.format == VK_FORMAT_B8G8R8A8_UNORM &&
				currentFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return currentFormat;
			}
		}

		return availableFormats[0];
	}

	VkPresentModeKHR PlanetVulkan::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes)
	{
		/*
		VK_PRESENT_MODE_IMMEDIATE_KHR
		Presents images as soon as possible. Gives highest frame rate but causes screen tearing.
		VK_PRESENT_MODE_MAILBOX_KHR
		When a new image is presented, it waits to display until the next vertical refresh. If a new image is presented before the refresh, it replaces the old image.
		VK_PRESENT_MODE_FIFO_KHR
		Images are stored in a queue and shown in order after each vertical refresh.
		VK_PRESENT_MODE_FIFO_RELAXED_KHR
		Functions just like standard FIFO, except if the queue is empty when there is a vertical refresh, the next image that is put into the queue will be presented immediately. As a result, also can cause screen tearing.
		*/

		for (const auto& currentMode : availablePresentModes)
		{
			if (currentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				return currentMode;
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D PlanetVulkan::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		{
			return capabilities.currentExtent;
		}
		else
		{
			VkExtent2D actualExtent = { windowObj.windowWidth, windowObj.windowHeight };
			actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
			actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));
			return actualExtent;
		}
	}
	
}

