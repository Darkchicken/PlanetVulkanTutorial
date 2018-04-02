#include "PVUniformBuffer.h"

namespace PVEngine
{

	PVUniformBuffer::PVUniformBuffer(const VkDevice* logicalDevice, const VkPhysicalDevice* physicalDevice, const VkSurfaceKHR* surface,
		const VkCommandPool* transferCommandPool, const VkQueue* transferQueue)
	{
		CreateUniformBuffer(logicalDevice, physicalDevice, surface, transferCommandPool, transferQueue);
	}


	PVUniformBuffer::~PVUniformBuffer()
	{
	}

	void PVUniformBuffer::CreateUniformBuffer(const VkDevice* logicalDevice, const VkPhysicalDevice* physicalDevice, const VkSurfaceKHR* surface,
		const VkCommandPool* transferCommandPool, const VkQueue* transferQueue)
	{
		VkDeviceSize bufferSize = GetUniformBufferSize();

		createBuffer(logicalDevice, physicalDevice, surface, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,buffer, bufferMemory);
	}
	void PVUniformBuffer::CleanupUniformBuffer(const VkDevice* logicalDevice)
	{
		cleanupBuffer(logicalDevice, buffer, bufferMemory);
	}

	void PVUniformBuffer::Update(const VkDevice* logicalDevice, const VkExtent2D &swapChainExtent)
	{
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		UniformBufferObject ubo = {};
		ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

		ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

		ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float) swapChainExtent.height, 0.1f, 10.0f);
		ubo.proj[1][1] *= -1; //Flipping the y cooridinate since glm projection view is left handed

		void* data;
		vkMapMemory(*logicalDevice, bufferMemory, 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(*logicalDevice, bufferMemory);
	}
}
