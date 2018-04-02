#include "PlanetVulkan.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <cstring>
#include <vector>
#include <map>
#include <algorithm>
#include <set>
#include "PVVertex.h"

namespace PVEngine
{

	PlanetVulkan::PlanetVulkan()
	{
	}


	PlanetVulkan::~PlanetVulkan()
	{
		delete swapchain;
		delete graphicsCommandPool;
		delete transferCommandPool;
		delete uniformBuffer;
		delete indexBuffer;
		delete vertexBuffer;
	}

	void PlanetVulkan::InitVulkan()
	{
		InitWindow();
		CreateInstance();
		SetupDebugCallback();
		CreateSurface();
		GetPhysicalDevices();
		CreateLogicalDevice();
		swapchain = new PVSwapchain();
		swapchain->Create(&logicalDevice, &physicalDevice, &surface, &windowObj, QuerySwapChainSupport(physicalDevice));
		CreateRenderPass();
		CreateDescriptorSetlayout();
		CreateGraphicsPipeline();
		swapchain->CreateFramebuffers(&renderPass);
		
		QueueFamilyIndices indices = FindQueueFamilies(&physicalDevice, &surface);
		graphicsCommandPool = new PVCommandPool(&logicalDevice, indices.graphicsFamily);
		transferCommandPool = new PVCommandPool(&logicalDevice, indices.transferFamily , VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);

		vertexBuffer = new PVVertexBuffer(&logicalDevice, &physicalDevice, &surface, transferCommandPool->GetCommandPool(), &transferQueue);
		indexBuffer = new PVIndexBuffer(&logicalDevice, &physicalDevice, &surface, transferCommandPool->GetCommandPool(), &transferQueue);
		uniformBuffer = new PVUniformBuffer(&logicalDevice, &physicalDevice, &surface, transferCommandPool->GetCommandPool(), &transferQueue);

		CreateDescriptorPool();
		CreateDescriptorSet();

		CreateCommandBuffers();
		CreateSemaphores();
	}

	void PlanetVulkan::CleanupVulkan()
	{
		CleanupSwapChain();

		vkDestroyDescriptorPool(logicalDevice, descriptorPool, nullptr);

		vkDestroyDescriptorSetLayout(logicalDevice, descriptorSetlayout, nullptr);

		uniformBuffer->CleanupUniformBuffer(&logicalDevice);

		indexBuffer->CleanupIndexBuffer(&logicalDevice);

		vertexBuffer->Cleanup(&logicalDevice);

		vkDestroySemaphore(logicalDevice, renderFinishedSemaphore, VK_NULL_HANDLE);
		vkDestroySemaphore(logicalDevice, imageAvailableSemaphore, VK_NULL_HANDLE);

		graphicsCommandPool->Cleanup(&logicalDevice);
		transferCommandPool->Cleanup(&logicalDevice);

		vkDestroyDevice(logicalDevice, VK_NULL_HANDLE);
		DestroyDebugReportCallbackEXT(instance, callback, VK_NULL_HANDLE);
		vkDestroySurfaceKHR(instance, surface, VK_NULL_HANDLE);
		vkDestroyInstance(instance, VK_NULL_HANDLE);
		glfwDestroyWindow(windowObj.window);
		glfwTerminate();
	}

	void PlanetVulkan::CleanupSwapChain()
	{
		swapchain->CleanupFramebuffers();

		vkFreeCommandBuffers(logicalDevice, *graphicsCommandPool->GetCommandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());

		vkDestroyPipeline(logicalDevice, graphicsPipeline, VK_NULL_HANDLE);
		vkDestroyPipelineLayout(logicalDevice, pipelineLayout, VK_NULL_HANDLE);
		vkDestroyRenderPass(logicalDevice, renderPass, VK_NULL_HANDLE);

		swapchain->Cleanup();
	}

	void PlanetVulkan::GameLoop()
	{
		while (!glfwWindowShouldClose(windowObj.window))
		{
			glfwPollEvents();

			//Update transformation matrices
			uniformBuffer->Update(&logicalDevice, *swapchain->GetExtent());

			DrawFrame();
		}

		vkDeviceWaitIdle(logicalDevice);
		CleanupVulkan();
	}

	void PlanetVulkan::InitWindow()
	{
		windowObj.Create();
		glfwSetWindowUserPointer(windowObj.window, this);
		glfwSetWindowSizeCallback(windowObj.window,PlanetVulkan::OnWindowResized);
	}

	void PlanetVulkan::RecreateSwapChain()
	{
		vkDeviceWaitIdle(logicalDevice);
		CleanupSwapChain();

		swapchain->Create(&logicalDevice, &physicalDevice, &surface, &windowObj, QuerySwapChainSupport(physicalDevice));
		CreateRenderPass();
		CreateGraphicsPipeline();
		swapchain->CreateFramebuffers(&renderPass);
		CreateCommandBuffers();
	}

	void PlanetVulkan::CreateInstance()
	{

		//check for validation layer availability
		if (enableValidationLayers && !CheckValidationLayerSupport())
		{
			throw std::runtime_error("Validation layers requested but not available");
		}
		else
		{
			std::cout << "Requested layers available" << std::endl;
		}


		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pNext = nullptr;
		appInfo.pApplicationName = "Planet Vulkan";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.pApplicationInfo = &appInfo;

		if (enableValidationLayers)
		{
			createInfo.enabledLayerCount = validationLayers.size();
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else
		{
			createInfo.enabledLayerCount = 0;
			createInfo.ppEnabledLayerNames = nullptr;
		}

		auto extensions = GetRequiredExtensions();
		createInfo.enabledExtensionCount = extensions.size();
		createInfo.ppEnabledExtensionNames = extensions.data();

		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create instance!");
		}
		else
		{
			std::cout << "Vulkan instance create successfully" << std::endl;
		}
	}

	bool PlanetVulkan::CheckValidationLayerSupport()
	{
		std::cout << "Checking Support..." << std::endl;
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName : validationLayers)
		{
			bool layerFound = false;
			for (const auto &layerProperties : availableLayers)
			{
				if (strcmp(layerName, layerProperties.layerName) == 0)
				{
					layerFound = true;
					break;
				}
			}
			if (!layerFound)
			{
				return false;
			}
		}
		return true;
	}

	std::vector<const char*> PlanetVulkan::GetRequiredExtensions()
	{
		std::vector<const char*> extensions;

		unsigned int glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		for (unsigned int i = 0; i < glfwExtensionCount; i++)
		{
			extensions.push_back(glfwExtensions[i]);
		}

		if (enableValidationLayers)
		{
			extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
		}

		return extensions;
	}

	void PlanetVulkan::SetupDebugCallback()
	{
		if (!enableValidationLayers)
		{
			return;
		}

		VkDebugReportCallbackCreateInfoEXT createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
		createInfo.pfnCallback = debugCallback;

		if (CreateDebugReportCallbackEXT(instance, &createInfo, nullptr, &callback) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to set up debug callback");
		}
		else
		{
			std::cout << "Debug Callback setup successful" << std::endl;
		}
	}

	void PlanetVulkan::CreateSurface()
	{
		if (glfwCreateWindowSurface(instance, windowObj.window, nullptr, &surface) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create window surface");
		}
		else
		{
			std::cout << "Window surface created successfully" << std::endl;
		}
	}

	void PlanetVulkan::GetPhysicalDevices()
	{
		uint32_t physicalDeviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);
		if (physicalDeviceCount == 0)
		{
			throw std::runtime_error("No devices found with Vulkan supprt");
		}

		std::vector<VkPhysicalDevice> foundPhysicalDevices(physicalDeviceCount);
		vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, foundPhysicalDevices.data());

		std::multimap<int, VkPhysicalDevice> rankedDevices;

		for (const auto& currentDevice : foundPhysicalDevices)
		{
			int score = RateDeviceSuitability(currentDevice);
			rankedDevices.insert(std::make_pair(score, currentDevice));
		}

		if (rankedDevices.rbegin()->first > 0)
		{
			physicalDevice = rankedDevices.rbegin()->second;
			std::cout << "Physical device found" << std::endl;
		}
		else
		{
			throw std::runtime_error("No physical devices meet necessary criteria");
		}
	}

	SwapChainSupportDetails PlanetVulkan::QuerySwapChainSupport(VkPhysicalDevice device)
	{
		SwapChainSupportDetails details;

		// capabilities
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

		//formats
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

		if (formatCount != 0)
		{
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
		}

		//presentModes
		uint32_t presentModesCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModesCount, nullptr);
		if (presentModesCount != 0)
		{
			details.presentModes.resize(presentModesCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModesCount, details.presentModes.data());
		}

		return details;
	}

	int PlanetVulkan::RateDeviceSuitability(VkPhysicalDevice deviceToRate)
	{
		int score = 0;

		QueueFamilyIndices indices = FindQueueFamilies(&deviceToRate, &surface);
		bool extensionsSupported = CheckDeviceExtensionSupport(deviceToRate);
		if (!indices.isComplete() || !extensionsSupported)
		{
			return 0;
		}

		bool swapChainAdequate = false;
		SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(deviceToRate);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		if (!swapChainAdequate)
		{
			return 0;
		}


		VkPhysicalDeviceFeatures deviceFeatures;
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(deviceToRate, &deviceProperties);
		vkGetPhysicalDeviceFeatures(deviceToRate, &deviceFeatures);

		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			score += 1000;
		}

		score += deviceProperties.limits.maxImageDimension2D;

		if (!deviceFeatures.geometryShader)
		{
			return 0;
		}

		return score;
	}



	void PlanetVulkan::CreateLogicalDevice()
	{
		QueueFamilyIndices indices = FindQueueFamilies(&physicalDevice, &surface);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<int> uniqueQueueFamilies = { indices.graphicsFamily, indices.transferFamily };
		const float queuePriority = 1.0f;
		for (int queueFamily : uniqueQueueFamilies)
		{
			VkDeviceQueueCreateInfo queueCreateInfo = {};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.pNext = nullptr;
			queueCreateInfo.flags = 0;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}
		

		VkPhysicalDeviceFeatures deviceFeatures = {};

		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.queueCreateInfoCount = 1;
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		if (enableValidationLayers)
		{
			createInfo.enabledLayerCount = validationLayers.size();
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else
		{
			createInfo.enabledLayerCount = 0;
			createInfo.ppEnabledLayerNames = nullptr;
		}
		createInfo.enabledExtensionCount = deviceExtensions.size();
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();
		createInfo.pEnabledFeatures = &deviceFeatures;

		if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &logicalDevice) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create logical device");
		}
		else
		{
			std::cout << "Logical device created successfully" << std::endl;
		}

		vkGetDeviceQueue(logicalDevice, indices.graphicsFamily, 0, &graphicsQueue);
		vkGetDeviceQueue(logicalDevice, indices.transferFamily, 0, &transferQueue);

	}



	void PlanetVulkan::CreateRenderPass()
	{
		VkAttachmentDescription colorAttachment = {};
		colorAttachment.format = *swapchain->GetImageFormat();
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(logicalDevice, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create render pass");
		}
		else
		{
			std::cout << "Render Pass created successfully" << std::endl;
		}
	}

	void PlanetVulkan::CreateDescriptorSetlayout()
	{
		VkDescriptorSetLayoutBinding uboLayoutBinding = {};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = 1;
		layoutInfo.pBindings = &uboLayoutBinding;

		if (vkCreateDescriptorSetLayout(logicalDevice, &layoutInfo, nullptr, &descriptorSetlayout) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create descriptor set layout");
		}
		else
		{
			std::cout << "Descriptor Set Layout created successfully" << std::endl;
		}
	}

	void PlanetVulkan::CreateDescriptorPool()
	{
		VkDescriptorPoolSize poolSize = {};
		poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSize.descriptorCount = 1;

		VkDescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = 1;
		poolInfo.pPoolSizes = &poolSize;
		poolInfo.maxSets = 1;

		if (vkCreateDescriptorPool(logicalDevice, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create descriptor pool");
		}
		else
		{
			std::cout << "Descriptor Pool created successfully" << std::endl;
		}
	}

	void PlanetVulkan::CreateDescriptorSet()
	{
		VkDescriptorSetLayout layouts[] = { descriptorSetlayout };
		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = layouts;
		if(vkAllocateDescriptorSets(logicalDevice, &allocInfo, &descriptorSet) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create descriptor set");
		}
		else
		{
			std::cout << "Descriptor Set created successfully" << std::endl;
		}
		VkDescriptorBufferInfo bufferInfo = {};
		bufferInfo.buffer = *uniformBuffer->GetBuffer();
		bufferInfo.offset = 0;
		bufferInfo.range = uniformBuffer->GetUniformBufferSize();

		VkWriteDescriptorSet descriptorWrite = {};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = descriptorSet;
		descriptorWrite.dstBinding = 0;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = &bufferInfo;

		vkUpdateDescriptorSets(logicalDevice, 1, &descriptorWrite, 0, nullptr);
	}

	void PlanetVulkan::CreateGraphicsPipeline()
	{
		auto vertShaderCode = ReadFile("Shaders/vert.spv");
		auto fragShaderCode = ReadFile("Shaders/frag.spv");

		VkShaderModule vertShaderModule;
		VkShaderModule fragShaderModule;

		vertShaderModule = CreateShaderModule(vertShaderCode);
		fragShaderModule = CreateShaderModule(fragShaderCode);

		VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		auto bindingDescription = Vertex::getBindingDescription();
		auto attributeDescriptions = Vertex::getAttributeDescriptions();
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = false;

		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)swapchain->GetExtent()->width;
		viewport.height = (float)swapchain->GetExtent()->height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.offset = { 0,0 };
		scissor.extent = *swapchain->GetExtent();

		VkPipelineViewportStateCreateInfo viewportState = {};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		VkPipelineRasterizationStateCreateInfo rasterizer = {};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;

		VkPipelineMultisampleStateCreateInfo multisampling = {};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlending = {};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;

		VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_LINE_WIDTH };

		VkPipelineDynamicStateCreateInfo dynamicState = {};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = 2;
		dynamicState.pDynamicStates = dynamicStates;

		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &descriptorSetlayout;

		if (vkCreatePipelineLayout(logicalDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create pipeline layout");
		}
		else
		{
			std::cout << "Pipeline layout created successfully" << std::endl;
		}

		VkGraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.layout = pipelineLayout;
		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = 0;


		if (vkCreateGraphicsPipelines(logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create graphics pipeline");
		}
		else
		{
			std::cout << "Graphics pipeline created successfully" << std::endl;
		}

		vkDestroyShaderModule(logicalDevice, vertShaderModule, VK_NULL_HANDLE);
		vkDestroyShaderModule(logicalDevice, fragShaderModule, VK_NULL_HANDLE);


	}

	VkShaderModule PlanetVulkan::CreateShaderModule(const std::vector<char>& code)
	{

		VkShaderModuleCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();

		std::vector<uint32_t> codeAligned(code.size() / sizeof(uint32_t) + 1);
		memcpy(codeAligned.data(), code.data(), code.size());
		createInfo.pCode = codeAligned.data();

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(logicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create shader module");
		}
		else
		{
			std::cout << "Shader module created successfully!" << std::endl;
		}

		return shaderModule;
	}


	void PlanetVulkan::CreateCommandBuffers()
	{
		commandBuffers.resize(swapchain->GetFramebufferSize());

		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = *graphicsCommandPool->GetCommandPool();
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

		if (vkAllocateCommandBuffers(logicalDevice, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create command buffers");
		}
		else
		{
			std::cout << "Command Buffers created successfully" << std::endl;
		}

		for (size_t i = 0; i < commandBuffers.size(); i++)
		{
			VkCommandBufferBeginInfo beginInfo = {};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

			vkBeginCommandBuffer(commandBuffers[i], &beginInfo);

			VkRenderPassBeginInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = renderPass;
			renderPassInfo.framebuffer = *swapchain->GetFramebuffer(i);
			renderPassInfo.renderArea.offset = { 0,0 };
			renderPassInfo.renderArea.extent = *swapchain->GetExtent();
			VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
			renderPassInfo.clearValueCount = 1;
			renderPassInfo.pClearValues = &clearColor;

			vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

			VkBuffer vertexBuffers[] = { *vertexBuffer->GetBuffer() };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);

			VkBuffer indexBfr = *indexBuffer->GetBuffer();
			vkCmdBindIndexBuffer(commandBuffers[i], indexBfr, 0, VK_INDEX_TYPE_UINT32);

			vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);

			vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(indexBuffer->GetIndicesSize()), 1, 0, 0, 0);

			vkCmdEndRenderPass(commandBuffers[i]);

			if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to record command buffer");
			}
		}
	}

	void PlanetVulkan::CreateSemaphores()
	{

		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		if (vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS
			|| vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create semaphores");
		}
		else
		{
			std::cout << "Semaphores created successfully" << std::endl;
		}
	}

	void PlanetVulkan::DrawFrame()
	{
		uint32_t imageIndex;
		vkAcquireNextImageKHR(logicalDevice, *swapchain->GetSwapchain(), std::numeric_limits<uint64_t>::max(), imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		VkSemaphore waitSemaphores[] = { imageAvailableSemaphore };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		VkSemaphore signalSemaphores[] = { renderFinishedSemaphore };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

		if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to submit draw command buffer");
		}

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapchains[] = { *swapchain->GetSwapchain() };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapchains;
		presentInfo.pImageIndices = &imageIndex;

		vkQueuePresentKHR(graphicsQueue, &presentInfo);
	}

	bool PlanetVulkan::CheckDeviceExtensionSupport(VkPhysicalDevice device)
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		for (const char* currentExtension : deviceExtensions)
		{
			bool extensionFound = false;
			for (const auto& extention : availableExtensions)
			{
				if (strcmp(currentExtension, extention.extensionName) == 0)
				{
					extensionFound = true;
					break;
				}
			}
			if (!extensionFound)
			{
				return false;
			}
		}
		return true;

	}
}

	
