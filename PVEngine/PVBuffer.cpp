#include "PVBuffer.h"
#include "PVQueueFamily.h"

namespace PVEngine
{
	PVBuffer::PVBuffer()
	{
	}


	PVBuffer::~PVBuffer()
	{
	}

	void PVBuffer::createBuffer(const VkDevice* logicalDevice, const VkPhysicalDevice* physicalDevice, 
		const VkSurfaceKHR* surface, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, 
		VkBuffer& buffer, VkDeviceMemory& bufferMemory)
	{
		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		

		QueueFamilyIndices indices = FindQueueFamilies(physicalDevice, surface);
		if (indices.graphicsFamily == indices.transferFamily)
		{
			bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			uint32_t indicesArray[] = { static_cast<uint32_t>(indices.graphicsFamily)};
			bufferInfo.pQueueFamilyIndices = indicesArray;
			bufferInfo.queueFamilyIndexCount = 1;
		}
		else
		{
			bufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
			uint32_t indicesArray[] = { static_cast<uint32_t>(indices.graphicsFamily), static_cast<uint32_t>(indices.transferFamily) };
			bufferInfo.pQueueFamilyIndices = indicesArray;
			bufferInfo.queueFamilyIndexCount = 2;
		}
		if (vkCreateBuffer(*logicalDevice, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create buffer");
		}
		else
		{
			std::cout << "Buffer created successfully" << std::endl;
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(*logicalDevice, buffer, &memRequirements);

		VkMemoryAllocateInfo allocateInfo = {};
		allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocateInfo.allocationSize = memRequirements.size;
		allocateInfo.memoryTypeIndex = findMemoryType(*physicalDevice, memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(*logicalDevice, &allocateInfo, nullptr, &bufferMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate buffer memory");
		}
		else
		{
			std::cout << "Buffer memory allocated successfully" << std::endl;
		}

		vkBindBufferMemory(*logicalDevice, buffer, bufferMemory, 0);
	}

	void PVBuffer::cleanupBuffer(const VkDevice* logicalDevice, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
	{
		vkDestroyBuffer(*logicalDevice, buffer, nullptr);
		vkFreeMemory(*logicalDevice, bufferMemory, nullptr);
	}

	uint32_t PVBuffer::findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i)
		{
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}

		throw std::runtime_error("Failed to find suitable memory type");
	}

	void PVBuffer::copyBuffer(const VkDevice* logicalDevice, const VkCommandPool* commandPool, VkBuffer srcBuffer,
		VkBuffer dstBuffer, VkDeviceSize size, const VkQueue* queue)
	{
		VkCommandBufferAllocateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		bufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		bufferInfo.commandPool = *commandPool;
		bufferInfo.commandBufferCount = 1;
		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(*logicalDevice, &bufferInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		VkBufferCopy copyRegion = {};
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;
		vkQueueSubmit(*queue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(*queue);

		vkFreeCommandBuffers(*logicalDevice, *commandPool, 1, &commandBuffer);
	}
}
