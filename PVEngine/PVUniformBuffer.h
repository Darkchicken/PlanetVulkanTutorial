#pragma once

#include "PVBuffer.h"

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>

namespace PVEngine
{
	class PVUniformBuffer : public PVBuffer
	{
	public:

		struct UniformBufferObject
		{
			glm::mat4 model;
			glm::mat4 view;
			glm::mat4 proj;
		};


		PVUniformBuffer(const VkDevice* logicalDevice, const VkPhysicalDevice* physicalDevice, const VkSurfaceKHR* surface,
			const VkCommandPool* transferCommandPool, const VkQueue* transferQueue);
		~PVUniformBuffer();

		void CreateUniformBuffer(const VkDevice* logicalDevice, const VkPhysicalDevice* physicalDevice, const VkSurfaceKHR* surface,
			const VkCommandPool* transferCommandPool, const VkQueue* transferQueue);
		void CleanupUniformBuffer(const VkDevice* logicalDevice);

		void Update(const VkDevice* logicalDevice, const VkExtent2D &swapChainExtent);

		//Getters
		VkDeviceSize GetUniformBufferSize() { return sizeof(UniformBufferObject); }

	private:

	
	};
}

