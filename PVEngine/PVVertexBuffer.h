#pragma once
#include "PVVertex.h"
#include <iostream>
#include <vector>
namespace PVEngine
{
	class PVVertexBuffer
	{
	public:
		PVVertexBuffer();
		~PVVertexBuffer();

		//Getters
		VkBuffer* GetVertexBuffer() { return &vertexBuffer; }
		uint32_t GetVerticesSize() { return vertices.size(); }

		void Create(const VkDevice* logicalDevice, const VkPhysicalDevice* physicalDevice);
		void Cleanup(const VkDevice* logicalDevice);

	private:
		uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

		VkBuffer vertexBuffer;
		VkDeviceMemory vertexBufferMemory;

		const std::vector<Vertex> vertices = 
		{
			{ {0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
			{ { 0.5f, 0.5f },{ 0.0f, 1.0f, 0.0f } },
			{ {-0.5f, 0.5f},{ 0.0f, 0.0f, 1.0f } }
		};
	};
}

