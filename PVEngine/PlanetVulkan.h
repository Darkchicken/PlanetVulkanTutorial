#pragma once
#include <vulkan/vulkan.h>
#include "VDeleter.h"
namespace PVEngine
{
	class PlanetVulkan
	{
	public:
		PlanetVulkan();
		~PlanetVulkan();

		void InitVulkan();

	private:
		void CreateInstance();

		VDeleter<VkInstance> instance { vkDestroyInstance };
	};
}

