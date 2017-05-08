#pragma once
#include <vulkan/vulkan.h>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <fstream>

#include "Window.h"
#include "VDeleter.h"

namespace PVEngine
{

	static std::vector<char> ReadFile(const std::string& filename)
	{
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open())
		{
			throw std::runtime_error("Failed to open file");
		}

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);
		file.close();

		return buffer;
	}


	struct QueueFamilyIndices
	{
		int displayFamily = -1;
		
		bool isComplete()
		{
			return displayFamily >= 0;
		}
	};

	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	class PlanetVulkan
	{
	public:
		PlanetVulkan();
		~PlanetVulkan();

		void InitVulkan();

		void GameLoop();

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

		void CreateSwapChain();

		void CreateImageViews();

		void CreateRenderPass();

		void CreateGraphicsPipeline();

		void CreateShaderModule(const std::vector<char>& code, VDeleter<VkShaderModule>& shaderModule);

		void CreateFramebuffers();

		void CreateCommandPool();
		
		void CreateCommandBuffers();

		void CreateSemaphores();

		void DrawFrame();

		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);

		bool CheckDeviceExtensionSupport(VkPhysicalDevice device);

		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);

		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);

		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);


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

		const std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

		///Vulkan Handles
		VDeleter<VkInstance> instance { vkDestroyInstance };

		VDeleter<VkDebugReportCallbackEXT> callback {instance, DestroyDebugReportCallbackEXT };

		VDeleter<VkSurfaceKHR> surface{instance, vkDestroySurfaceKHR};

		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

		VDeleter<VkDevice> logicalDevice{ vkDestroyDevice };

		VkQueue displayQueue;

		VDeleter<VkSwapchainKHR> swapChain{logicalDevice, vkDestroySwapchainKHR};

		std::vector<VkImage> swapChainImages;

		std::vector<VDeleter<VkImageView>> swapChainImageViews;

		VDeleter<VkRenderPass> renderPass{logicalDevice, vkDestroyRenderPass};

		VDeleter<VkPipelineLayout> pipelineLayout{logicalDevice, vkDestroyPipelineLayout};

		VDeleter<VkPipeline> graphicsPipeline{logicalDevice, vkDestroyPipeline};

		std::vector<VDeleter<VkFramebuffer>> swapChainFramebuffers;

		VDeleter<VkCommandPool> commandPool{logicalDevice, vkDestroyCommandPool};

		std::vector<VkCommandBuffer> commandBuffers;

		VDeleter<VkSemaphore> imageAvailableSemaphore {logicalDevice, vkDestroySemaphore};

		VDeleter<VkSemaphore> renderFinishedSemaphore{ logicalDevice, vkDestroySemaphore };



		// store swap chain details
		VkFormat swapChainImageFormat;
		VkExtent2D swapChainExtent;

		

#ifdef NDEBUG
		const bool enableValidationLayers = false;
#else
		const bool enableValidationLayers = true;
#endif //NDEBUG
	};
}

