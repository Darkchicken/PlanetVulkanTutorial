#pragma once
#include <vulkan/vulkan.h>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "Window.h"
#include "VDeleter.h"

namespace PVEngine
{
	struct QueueFamilyIndices
	{
		int displayFamily = -1;
		
		bool isComplete()
		{
			return displayFamily >= 0;
		}
	};

	class PlanetVulkan
	{
	public:
		PlanetVulkan();
		~PlanetVulkan();

		void InitVulkan();

		Window windowObj;

	private:
		void CreateInstance();

		bool CheckValidationLayerSupport();

		std::vector<const char*> GetRequiredExtensions();

		void SetupDebugCallback();

		void CreateSurface();

		void GetPhysicalDevices();

		int RateDeviceSuitability(VkPhysicalDevice deviceToRate);

		void CreateLogicalDevice();

		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);


		VkResult CreateDebugReportCallbackEXT(
			VkInstance instance,
			const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
			const VkAllocationCallbacks* pAllocator,
			VkDebugReportCallbackEXT* pCallback
		)
		{
			auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
			if (func != nullptr)
			{
				return func(instance, pCreateInfo, pAllocator, pCallback);
			}
			else
			{
				return VK_ERROR_EXTENSION_NOT_PRESENT;
			}
		};

		static void DestroyDebugReportCallbackEXT(
			VkInstance instance,
			VkDebugReportCallbackEXT callback,
			const VkAllocationCallbacks* pAllocator
		)
		{
			auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
			if (func != nullptr)
			{
				func(instance, callback, pAllocator);
			}
		};
		
		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
			VkDebugReportFlagsEXT flags,
			VkDebugReportObjectTypeEXT objType,
			uint64_t obj,
			size_t location,
			int32_t code,
			const char* layerPrefix,
			const char* msg,
			void* userData
			) 
		{
			std::cerr << "Validation layer: " << msg << std::endl;
			return VK_FALSE;
		};
		const std::vector<const char*> validationLayers = { "VK_LAYER_LUNARG_standard_validation" };

		///Vulkan Handles
		VDeleter<VkInstance> instance { vkDestroyInstance };

		VDeleter<VkDebugReportCallbackEXT> callback {instance, DestroyDebugReportCallbackEXT };

		VDeleter<VkSurfaceKHR> surface{instance, vkDestroySurfaceKHR};

		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

		VDeleter<VkDevice> logicalDevice{ vkDestroyDevice };

		VkQueue displayQueue;

		

#ifdef NDEBUG
		const bool enableValidationLayers = false;
#else
		const bool enableValidationLayers = true;
#endif //NDEBUG
	};
}

