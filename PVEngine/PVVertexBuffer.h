#pragma once

#include "PVBuffer.h"
namespace PVEngine
{
	class PVVertexBuffer : public PVBuffer
	{
	public:
		PVVertexBuffer(const VkDevice* logicalDevice, const VkPhysicalDevice* physicalDevice, const VkSurfaceKHR* surface,
			const VkCommandPool* transferCommandPool, const VkQueue* transferQueue);
		~PVVertexBuffer();

		void Create(const VkDevice* logicalDevice, const VkPhysicalDevice* physicalDevice, const VkSurfaceKHR* surface,
			const VkCommandPool* transferCommandPool, const VkQueue* transferQueue);
		void Cleanup(const VkDevice* logicalDevice);

		//Getters
		uint32_t GetVerticesSize() { return vertices.size(); }
	private:

		const std::vector<Vertex> vertices = 
		{
			{ { -0.5f, -0.5f },{ 1.0f, 1.0f, 1.0f } },
			{ {0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
			{ { 0.5f, 0.5f },{ 0.0f, 1.0f, 0.0f } },
			{ {-0.5f, 0.5f},{ 0.0f, 0.0f, 1.0f } }
		};
	};
}

