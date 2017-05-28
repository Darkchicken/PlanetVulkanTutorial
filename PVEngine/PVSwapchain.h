#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <iostream>
#include <stdexcept>

#include "Window.h"

namespace PVEngine
{

	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	class PVSwapchain
	{
	public:
		PVSwapchain();
		~PVSwapchain();

		void Create(const VkDevice* logicalDevice, VkPhysicalDevice* physicalDevice, const VkSurfaceKHR* surface, Window* windowObj, SwapChainSupportDetails swapChainSupport);
		void Cleanup();
		void CleanupFramebuffers();


		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);

		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, Window windowObj);

		void CreateFramebuffers(VkRenderPass* renderPass);

		void CreateImageViews();

		//Getters
		VkSwapchainKHR* GetSwapchain() { return &swapChain; }
		VkFormat* GetImageFormat() { return &swapChainImageFormat; }
		VkExtent2D* GetExtent() { return &swapChainExtent; }
		size_t GetFramebufferSize() { return swapChainFramebuffers.size(); }
		VkFramebuffer* GetFramebuffer(unsigned int index) { return &swapChainFramebuffers[index]; }


	private:
		VkSwapchainKHR swapChain;

		std::vector<VkImage> swapChainImages;

		std::vector<VkImageView> swapChainImageViews;

		std::vector<VkFramebuffer> swapChainFramebuffers;

		// store swap chain details
		VkFormat swapChainImageFormat;
		VkExtent2D swapChainExtent;

		const VkDevice* device;
	};
}

