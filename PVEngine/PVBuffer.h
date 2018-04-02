#pragma once
#include "PVVertex.h"
#include <iostream>
#include <vector>
namespace PVEngine
{
	class PVBuffer
	{
	public:
		PVBuffer();
		~PVBuffer();

		VkBuffer* GetBuffer() { return &buffer; }

	protected:
		void createBuffer(const VkDevice* logicalDevice, const VkPhysicalDevice* physicalDevice,
			const VkSurfaceKHR* surface, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, 
			VkBuffer& buffer, VkDeviceMemory& bufferMemory);

		void cleanupBuffer(const VkDevice* logicalDevice, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

		uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

		void copyBuffer(const VkDevice* logicalDevice, const VkCommandPool* commandPool, VkBuffer srcBuffer, 
			VkBuffer dstBuffer, VkDeviceSize size, const VkQueue* queue);

	protected:
		VkBuffer buffer;
		VkDeviceMemory bufferMemory;
	};
}

