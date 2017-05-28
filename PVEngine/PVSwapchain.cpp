#include "PVSwapchain.h"

#include <algorithm>

namespace PVEngine
{

	PVSwapchain::PVSwapchain()
	{
	}


	PVSwapchain::~PVSwapchain()
	{
	}

	void PVSwapchain::Create(const VkDevice* logicalDevice, VkPhysicalDevice* physicalDevice, const VkSurfaceKHR* surface, Window* windowObj,SwapChainSupportDetails swapChainSupport)
	{
		device = logicalDevice;
		// use helper functions to get optimal settings
		VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities, *windowObj);

		// fill in data fro create info
		VkSwapchainCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = *surface;

		// get proper image count 
		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
		{
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;



		//attempt to create swap chain
		if (vkCreateSwapchainKHR(*logicalDevice, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create swap chain");
		}
		else
		{
			std::cout << "Swap chain created successfully" << std::endl;
		}

		// populate swap chain image vector
		vkGetSwapchainImagesKHR(*logicalDevice, swapChain, &imageCount, nullptr);
		swapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(*logicalDevice, swapChain, &imageCount, swapChainImages.data());

		// stores data for chosen surface format and extent
		swapChainImageFormat = surfaceFormat.format;
		swapChainExtent = extent;

		CreateImageViews();
	}

	void PVSwapchain::Cleanup()
	{
		for (size_t i = 0; i < swapChainImageViews.size(); i++)
		{
			vkDestroyImageView(*device, swapChainImageViews[i], VK_NULL_HANDLE);
		}
		vkDestroySwapchainKHR(*device, swapChain, VK_NULL_HANDLE);
	}

	void PVSwapchain::CleanupFramebuffers()
	{
		for (size_t i = 0; i < swapChainFramebuffers.size(); i++)
		{
			vkDestroyFramebuffer(*device, swapChainFramebuffers[i], VK_NULL_HANDLE);
		}
	}

	void PVSwapchain::CreateImageViews()
	{
		swapChainImageViews.resize(swapChainImages.size());

		for (uint32_t i = 0; i<swapChainImages.size(); i++)
		{
			VkImageViewCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = swapChainImages[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = swapChainImageFormat;
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(*device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create image views");
			}
		}

		std::cout << "Image views created successfully" << std::endl;
	}

	void PVSwapchain::CreateFramebuffers(VkRenderPass* renderPass)
	{
		swapChainFramebuffers.resize(swapChainImageViews.size());

		for (size_t i = 0; i < swapChainImageViews.size(); i++)
		{
			VkImageView attachments[] = { swapChainImageViews[i] };


			VkFramebufferCreateInfo framebufferInfo = {};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = *renderPass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = swapChainExtent.width;
			framebufferInfo.height = swapChainExtent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(*device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create framebuffer");
			}
		}

		std::cout << "Framebuffers created successfully!" << std::endl;
	}

	

	VkSurfaceFormatKHR PVSwapchain::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		// if surface has no preferred format
		if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED)
		{
			return{ VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
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

	VkPresentModeKHR PVSwapchain::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes)
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

	VkExtent2D PVSwapchain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, Window windowObj)
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

