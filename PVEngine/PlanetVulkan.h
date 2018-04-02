#pragma once
#include <vulkan/vulkan.h>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <fstream>

#include "Window.h"
#include "VDeleter.h"
#include "PVSwapchain.h"
#include "PVVertexBuffer.h"
#include "PVIndexBuffer.h"
#include "PVUniformBuffer.h"
#include "PVQueueFamily.h"
#include "PVCommandPool.h"

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

	class PlanetVulkan
	{
	public:
		PlanetVulkan();
		~PlanetVulkan();

		void InitVulkan();

		void CleanupVulkan();

		void GameLoop();

		Window windowObj;

	private:
		static void OnWindowResized(GLFWwindow* window, int width, int height)
		{
			if (width == 0 || height == 0)
				return;

			PlanetVulkan* engine = reinterpret_cast<PlanetVulkan*>(glfwGetWindowUserPointer(window));
			engine->RecreateSwapChain();
		}
		void InitWindow();

		void CreateInstance();

		void CleanupSwapChain();

		bool CheckValidationLayerSupport();

		std::vector<const char*> GetRequiredExtensions();

		void SetupDebugCallback();

		void CreateSurface();

		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);

		void GetPhysicalDevices();

		int RateDeviceSuitability(VkPhysicalDevice deviceToRate);

		void CreateLogicalDevice();

		void RecreateSwapChain();
		//void CreateSwapChain();


		void CreateRenderPass();

		void CreateDescriptorSetlayout();

		void CreateDescriptorPool();

		void CreateDescriptorSet();

		void CreateGraphicsPipeline();

		VkShaderModule CreateShaderModule(const std::vector<char>& code);
		
		void CreateCommandBuffers();

		void CreateSemaphores();

		void DrawFrame();

		bool CheckDeviceExtensionSupport(VkPhysicalDevice device);

		


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
		VkInstance instance;

		VkDebugReportCallbackEXT callback;

		VkSurfaceKHR surface;

		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

		VkDevice logicalDevice;

		VkQueue graphicsQueue;

		VkQueue transferQueue;

		PVSwapchain* swapchain;

		

		VkRenderPass renderPass;

		VkDescriptorSetLayout descriptorSetlayout;

		VkPipelineLayout pipelineLayout;

		VkPipeline graphicsPipeline;

		PVCommandPool* graphicsCommandPool;

		PVCommandPool* transferCommandPool;

		VkDescriptorPool descriptorPool;

		PVVertexBuffer* vertexBuffer;

		PVIndexBuffer* indexBuffer;

		PVUniformBuffer* uniformBuffer;

		std::vector<VkCommandBuffer> commandBuffers;

		VkSemaphore imageAvailableSemaphore;

		VkSemaphore renderFinishedSemaphore;

		VkDescriptorSet descriptorSet;



		

		

#ifdef NDEBUG
		const bool enableValidationLayers = false;
#else
		const bool enableValidationLayers = true;
#endif //NDEBUG
	};
}

