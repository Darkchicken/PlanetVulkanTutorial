#pragma once
#include <vulkan/vulkan.h>
#include <iostream>
namespace PVEngine
{
	class PVCommandPool
	{
	public:
		PVCommandPool(const VkDevice* logicalDevice, uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = 0);
		~PVCommandPool();

		void Cleanup(const VkDevice* logicalDevice);

		VkCommandPool* GetCommandPool() { return &commandPool; }
	private:
		void createCommandPool(const VkDevice* logicalDevice, uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = 0);
		VkCommandPool commandPool;
	};
}

