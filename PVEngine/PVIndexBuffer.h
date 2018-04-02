#pragma once

#include "PVBuffer.h"

namespace PVEngine
{
	class PVIndexBuffer : public PVBuffer
	{
	public:
		PVIndexBuffer(const VkDevice* logicalDevice, const VkPhysicalDevice* physicalDevice, const VkSurfaceKHR* surface,
			const VkCommandPool* transferCommandPool, const VkQueue* transferQueue);
		~PVIndexBuffer();

		void CreateIndexBuffer(const VkDevice* logicalDevice, const VkPhysicalDevice* physicalDevice, const VkSurfaceKHR* surface,
			const VkCommandPool* transferCommandPool, const VkQueue* transferQueue);
		void CleanupIndexBuffer(const VkDevice* logicalDevice);

		//Getters
		uint32_t GetIndicesSize() { return indices.size(); }

	private:

		const std::vector<uint32_t> indices =
		{
			0,1,2,2,3,0
		};
	};
}

