#include "PVQueueFamily.h"
#include <vector>

namespace PVEngine
{
	QueueFamilyIndices FindQueueFamilies(const VkPhysicalDevice* device, const VkSurfaceKHR* surface)
	{
		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(*device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(*device, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto &queueFamily : queueFamilies)
		{
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(*device, i, *surface, &presentSupport);

			if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT && presentSupport)
			{
				indices.graphicsFamily = i;
			}

			if (queueFamily.queueCount > 0 && (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) && !(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) && presentSupport)
			{
				indices.transferFamily = i;
			}

			if (indices.isComplete())
			{
				break;
			}
			i++;
		}

		if (indices.graphicsFamily >= 0 && indices.transferFamily == -1)
		{
			indices.transferFamily = indices.graphicsFamily;
		}

		return indices;
	}
}
