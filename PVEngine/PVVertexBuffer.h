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

		//Getters
		VkBuffer* GetBuffer() { return &vertexBuffer; }
		uint32_t GetVerticesSize() { return vertices.size(); }

		VkBuffer* GetIndexBuffer() { return &indexBuffer; }
		uint32_t GetIndicesSize() { return indices.size(); }

		void Create(const VkDevice* logicalDevice, const VkPhysicalDevice* physicalDevice, const VkSurfaceKHR* surface,
			const VkCommandPool* transferCommandPool, const VkQueue* transferQueue);
		void Cleanup(const VkDevice* logicalDevice);

		void CreateIndexBuffer(const VkDevice* logicalDevice, const VkPhysicalDevice* physicalDevice, const VkSurfaceKHR* surface,
			const VkCommandPool* transferCommandPool, const VkQueue* transferQueue);
		void CleanupIndexBuffer(const VkDevice* logicalDevice);

	private:

		const std::vector<Vertex> vertices = 
		{
			{ { -0.5f, -0.5f },{ 1.0f, 1.0f, 1.0f } },
			{ {0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
			{ { 0.5f, 0.5f },{ 0.0f, 1.0f, 0.0f } },
			{ {-0.5f, 0.5f},{ 0.0f, 0.0f, 1.0f } }
		};

		const std::vector<uint32_t> indices =
		{
			0,1,2,2,3,0
		};


		VkBuffer vertexBuffer;
		VkDeviceMemory vertexBufferMemory;

		VkBuffer indexBuffer;
		VkDeviceMemory indexBufferMemory;
	};
}

