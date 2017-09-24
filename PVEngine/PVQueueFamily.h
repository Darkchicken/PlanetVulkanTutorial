#pragma once
#include <vulkan/vulkan.h>

namespace PVEngine
{
	struct QueueFamilyIndices
	{
		int graphicsFamily = -1;
		int transferFamily = -1;

		bool isComplete()
		{
			return (graphicsFamily >= 0 && transferFamily >= 0);
		}
	};

	QueueFamilyIndices FindQueueFamilies(const VkPhysicalDevice* device, const VkSurfaceKHR* surface);
}

