#include "PVIndexBuffer.h"


namespace PVEngine
{
	PVIndexBuffer::PVIndexBuffer(const VkDevice* logicalDevice, const VkPhysicalDevice* physicalDevice, const VkSurfaceKHR* surface,
		const VkCommandPool* transferCommandPool, const VkQueue* transferQueue)
	{
		CreateIndexBuffer(logicalDevice, physicalDevice, surface, transferCommandPool, transferQueue);
	}


	PVIndexBuffer::~PVIndexBuffer()
	{
	}


	void PVIndexBuffer::CreateIndexBuffer(const VkDevice* logicalDevice, const VkPhysicalDevice* physicalDevice, const VkSurfaceKHR* surface,
		const VkCommandPool* transferCommandPool, const VkQueue* transferQueue)
	{
		VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		createBuffer(logicalDevice, physicalDevice, surface, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(*logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, indices.data(), (size_t)bufferSize);
		vkUnmapMemory(*logicalDevice, stagingBufferMemory);

		createBuffer(logicalDevice, physicalDevice, surface, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, bufferMemory);

		copyBuffer(logicalDevice, transferCommandPool, stagingBuffer, buffer, bufferSize, transferQueue);

		vkDestroyBuffer(*logicalDevice, stagingBuffer, nullptr);
		vkFreeMemory(*logicalDevice, stagingBufferMemory, nullptr);
	}
	void PVIndexBuffer::CleanupIndexBuffer(const VkDevice* logicalDevice)
	{
		cleanupBuffer(logicalDevice, buffer, bufferMemory);
	}
}
