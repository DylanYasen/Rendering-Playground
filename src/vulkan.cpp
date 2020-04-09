#include "vulkan/vulkan_core.h"
#include "SDL_vulkan.h"

#ifdef WIN32
#include <Windows.h>
#include "vulkan/vulkan_win32.h"
#endif

#include <unordered_set>
#include <fstream>
#include <cassert>

namespace Vulkan
{
	const int WIDTH = 800;
	const int HEIGHT = 600;

	VkInstance instance;
	VkDevice device;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	VkSurfaceKHR surface;
	VkSwapchainKHR swapChain;
	std::vector<VkImage> swapChainImages;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	std::vector<VkImageView> swapChainImageViews;
	VkShaderModule vertShaderModule;
	VkShaderModule fragShaderModule;
	VkPipelineLayout pipelineLayout;
	VkRenderPass renderPass;
	VkPipeline graphicsPipeline;
	std::vector<VkFramebuffer> swapchainFrameBuffers;

	VkCommandPool commandPool;
	std::vector<VkCommandBuffer> commandBuffers;

	const int MAX_FRAMES_IN_FLIGHT = 2;
	size_t currentFrame = 0;
	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
	std::vector<VkFence> imagesInFlight;

	bool bInited = false;

	struct QueueFamilyIndices
	{
		/*unsigned */
		int graphicsFamily;
		int presentFamily;
		// these two are the same most of the time

		bool isValid()
		{
			return graphicsFamily >= 0 && presentFamily >= 0;
		}
	};

	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	const char* deviceExtensions[] =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

#if DEBUG || _DEBUG
	const bool enableValidationLayers = true;
	const char* validationLayers[] =
	{
		"VK_LAYER_KHRONOS_validation"
	};
	VkDebugUtilsMessengerEXT debugMessenger;
#else
	const bool enableValidationLayers = false;
#endif

	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice physicalDevice)
	{
		SwapChainSupportDetails details = {};

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &details.capabilities);


		unsigned int formatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, NULL);
		if (formatCount > 0)
		{
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, details.formats.data());
		}

		unsigned int presentModeCount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, NULL);
		if (presentModeCount > 0) {
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	VkSurfaceFormatKHR chooseSwapChainFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		assert(availableFormats.size() > 0);
		// srgb
		//for (const auto& availableFormat : availableFormats) {
		//	if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
		//		return availableFormat;
		//	}
		//}

		return availableFormats[0];
	}

	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
	{
		// prefer triple buffering
		//for (const auto& availablePresentMode : availablePresentModes) {
		//	if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
		//		return availablePresentMode;
		//	}
		//}

		// FIFO is guaranteed to be available
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		if (capabilities.currentExtent.width != UINT32_MAX) {
			return capabilities.currentExtent;
		}
		else {
			VkExtent2D actualExtent = { WIDTH, HEIGHT };

			actualExtent.width = max(capabilities.minImageExtent.width, min(capabilities.maxImageExtent.width, actualExtent.width));
			actualExtent.height = max(capabilities.minImageExtent.height, min(capabilities.maxImageExtent.height, actualExtent.height));

			return actualExtent;
		}
	}

	bool checkValidationLayerSupport()
	{
		// todo: clean this shit up
		//unsigned int layerCount = 0;
		//vkEnumerateInstanceLayerProperties(&layerCount, 0);

		//VkLayerProperties* layers = (VkLayerProperties*)_malloca(sizeof(const char*) * (layerCount));
		//vkEnumerateInstanceLayerProperties(&layerCount, &layers[0]);

		//for (int i = 0; i < ARRAYSIZE(validationLayers); i++)
		//{
		//	bool layerFound = false;
		//	for (int j = 0; j < layerCount; j++)
		//	{
		//		const VkLayerProperties& layer = layers[j];
		//		if (strcmp(validationLayers[i], layer.layerName) == 0)
		//		{
		//			layerFound = true;
		//			break;
		//		}
		//	}

		//	if (!layerFound)
		//	{
		//		return false;
		//	}
		//}

		return true;
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT msgSeverity,
		VkDebugUtilsMessageTypeFlagsEXT msgType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		if (msgSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
			printf("vulkan debug msg: %s\n", pCallbackData->pMessage);
		}

		return VK_FALSE;
	}

	VkResult createDebugUtilsMessengerEXT(VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pDebugMessenger)
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func)
		{
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else
		{
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr) {
			func(instance, debugMessenger, pAllocator);
		}
	}

	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
	}

	void setupDebugMessenger()
	{
		if (!enableValidationLayers) return;
		VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
		populateDebugMessengerCreateInfo(createInfo);

		if (createDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
		{
			printf("failed to set up debug messenger\n");
		}
	}

	bool isDeviceSuitable(VkPhysicalDevice device)
	{
		// todo: i just want a fking triangle on the screen
		// check for presentation & graphical queue support
		// check for swap chain support
		return true;
	}

	static std::vector<char> readFile(const std::string filename)
	{
		std::ifstream file(filename, std::ios::ate | std::ios::binary);
		if (!file.is_open())
		{
			printf("failed to read file: %s\n", filename.c_str());
			return {};
		}

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);
		file.seekg(0);
		file.read(buffer.data(), fileSize);
		file.close();

		return buffer;
	}

	VkShaderModule createShaderModule(const std::vector<char>& code)
	{
		VkShaderModuleCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = (const uint32_t*)(code.data());

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(device, &createInfo, NULL, &shaderModule) != VK_SUCCESS)
		{
			printf("failed to create shader module");
		}

		return shaderModule;
	}

	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device)
	{
		QueueFamilyIndices indices = {};

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto& queueFamily : queueFamilies) {
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				indices.graphicsFamily = i;

				// todo: currently forcing these to be the same
				{
					VkBool32 presentSupport = false;
					vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
					if (presentSupport)
					{
						indices.presentFamily = i;
					}
				}
			}

			i++;
		}

		return indices;
	}

	void initVulkan(SDL_Window* window)
	{
		// optional data
		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.apiVersion = VK_API_VERSION_1_2;
		/*appInfo.pApplicationName = "";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);*/

		// required
		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		// query extension count
		unsigned int exCount = 0;
		if (!SDL_Vulkan_GetInstanceExtensions(window, &exCount, nullptr))
		{
			printf("failed to get vulkan extension count\n");
			return;
		}

		const char* requiredExtensions[] =
		{
#if DEBUG || _DEBUG
			VK_EXT_DEBUG_UTILS_EXTENSION_NAME
#endif
		};
		const char** exNames = (const char**)_malloca(sizeof(const char*) * (exCount + ARRAYSIZE(requiredExtensions)));

		// fill in the required extensions first
		for (int i = 0; i < ARRAYSIZE(requiredExtensions); i++)
		{
			exNames[i] = requiredExtensions[i];
		}

		if (!SDL_Vulkan_GetInstanceExtensions(window, &exCount, exNames + ARRAYSIZE(requiredExtensions)))
		{
			printf("failed to get vulkan extensions\n");
			return;
		}

		// append required extensions
		const size_t totalCount = exCount + ARRAYSIZE(requiredExtensions);
		for (int i = 0; i < totalCount; i++)
		{
			printf("%s\n", exNames[i]);
		}

		createInfo.enabledExtensionCount = totalCount;
		createInfo.ppEnabledExtensionNames = exNames;

		// debugger specific for logging msgs during instance creation
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
		if (enableValidationLayers)
		{
			if (!checkValidationLayerSupport())
			{
				printf("validation layers requested, but not available\n");
				return;
			}

			createInfo.enabledLayerCount = ARRAYSIZE(validationLayers);
			createInfo.ppEnabledLayerNames = validationLayers;

			populateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
		}
		else
		{
			createInfo.enabledLayerCount = 0;
			createInfo.ppEnabledLayerNames = 0;
			createInfo.pNext = 0;
		}

		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
		{
			printf("failed to create vulkan instance\n");
			return;
		}

		// needs a valid vk instance
		setupDebugMessenger();

		// create window surface
		// todo: this is win32 only, handle other platforms
		{
			VkWin32SurfaceCreateInfoKHR  createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
			createInfo.hwnd = GetActiveWindow();
			createInfo.hinstance = GetModuleHandle(NULL);

			if (vkCreateWin32SurfaceKHR(instance, &createInfo, 0, &surface) != VK_SUCCESS)
			{
				printf("failed to create window surface\n");
				return;
			}
		}

		// select physical device
		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
		{
			// query device count
			unsigned int deviceCount = 0;
			vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);

			if (deviceCount == 0)
			{
				printf("failed to find GPUs with Vulkan support\n");
				return;
			}

			VkPhysicalDevice* devices = (VkPhysicalDevice*)_malloca(sizeof(const char*) * (deviceCount));
			vkEnumeratePhysicalDevices(instance, &deviceCount, &devices[0]);

			if (deviceCount == 0)
			{
				printf("failed to find GPUs with Vulkan support\n");
				return;
			}

			// select a suitable device
			for (int i = 0; i < deviceCount; i++)
			{
				if (isDeviceSuitable(devices[i]))
				{
					physicalDevice = devices[i];
					break;
				}
			}

			if (physicalDevice == VK_NULL_HANDLE)
			{
				printf("failed to find suitable GPU with Vulkan support\n");
				return;
			}
		}

		// create logical device
		QueueFamilyIndices queueIndicies = findQueueFamilies(physicalDevice);
		{
			if (!queueIndicies.isValid())
			{
				printf("invalid graphics queue family\n");
				return;
			}

			VkDeviceQueueCreateInfo queueCreateInfo = {};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = (unsigned int)queueIndicies.graphicsFamily;
			queueCreateInfo.queueCount = 1;

			float queuePriority = 1.0f;
			queueCreateInfo.pQueuePriorities = &queuePriority;

			VkPhysicalDeviceFeatures deviceFeatures = {};

			{
				VkDeviceCreateInfo  createInfo = {};
				createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
				createInfo.pQueueCreateInfos = &queueCreateInfo;
				createInfo.queueCreateInfoCount = 1;

				createInfo.pEnabledFeatures = &deviceFeatures;

				createInfo.enabledExtensionCount = ARRAYSIZE(deviceExtensions);
				createInfo.ppEnabledExtensionNames = deviceExtensions;

				if (enableValidationLayers)
				{
					createInfo.enabledLayerCount = ARRAYSIZE(validationLayers);
					createInfo.ppEnabledLayerNames = validationLayers;
				}
				else
				{
					createInfo.enabledLayerCount = 0;
				}

				if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
				{
					printf("failed to create logical device\n");
					return;
				}
			}
			vkGetDeviceQueue(device, queueIndicies.graphicsFamily /*queueIndicies.presentFamily*/, 0, &graphicsQueue);
			vkGetDeviceQueue(device, queueIndicies.presentFamily /*queueIndicies.presentFamily*/, 0, &presentQueue);
		}


		// create swap chain
		{
			SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

			const VkSurfaceFormatKHR& surfaceFormat = chooseSwapChainFormat(swapChainSupport.formats);
			const VkPresentModeKHR& presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
			const VkExtent2D& extent = chooseSwapExtent(swapChainSupport.capabilities);

			unsigned int imageCount = min(swapChainSupport.capabilities.minImageCount + 1, swapChainSupport.capabilities.maxImageCount);

			VkSwapchainCreateInfoKHR createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
			createInfo.surface = surface;
			createInfo.minImageCount = imageCount;
			createInfo.imageFormat = surfaceFormat.format;
			createInfo.imageColorSpace = surfaceFormat.colorSpace;
			createInfo.imageExtent = extent;
			createInfo.imageArrayLayers = 1;
			createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;   // VK_IMAGE_USAGE_TRANSFER_DST_BIT for post processing

			// not worried about different queue families for now
			assert(queueIndicies.graphicsFamily == queueIndicies.presentFamily);
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

			createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
			createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
			createInfo.presentMode = presentMode;
			createInfo.clipped = VK_TRUE;
			createInfo.oldSwapchain = VK_NULL_HANDLE;

			if (vkCreateSwapchainKHR(device, &createInfo, NULL, &swapChain) != VK_SUCCESS)
			{
				printf("failed to create swap chain");
				return;
			}

			vkGetSwapchainImagesKHR(device, swapChain, &imageCount, NULL);
			swapChainImages.resize(imageCount);
			vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

			swapChainExtent = extent;
			swapChainImageFormat = surfaceFormat.format;
		}

		// create image views
		{
			swapChainImageViews.resize(swapChainImages.size());

			for (size_t i = 0; i < swapChainImages.size(); i++)
			{
				VkImageViewCreateInfo createInfo = {};
				createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				createInfo.image = swapChainImages[i];
				createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
				createInfo.format = swapChainImageFormat;
				createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
				createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
				createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
				createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

				createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				createInfo.subresourceRange.baseMipLevel = 0;
				createInfo.subresourceRange.levelCount = 1;
				createInfo.subresourceRange.baseArrayLayer = 0;
				createInfo.subresourceRange.layerCount = 1;

				if (vkCreateImageView(device, &createInfo, NULL, &swapChainImageViews[i]) != VK_SUCCESS)
				{
					printf("failed to create swap chain image view");
					return;
				}
			}
		}


		// create graphics pipeline
		{
			const auto& vertShaderCode = Vulkan::readFile("shaders/vert.spv");
			const auto& fragShaderCode = Vulkan::readFile("shaders/frag.spv");

			vertShaderModule = createShaderModule(vertShaderCode);
			fragShaderModule = createShaderModule(fragShaderCode);

			// shader stage creation
			VkPipelineShaderStageCreateInfo vertCreateInfo = {};
			vertCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			vertCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
			vertCreateInfo.module = vertShaderModule;
			vertCreateInfo.pName = "main";

			// frag
			VkPipelineShaderStageCreateInfo fragCreateInfo = {};
			fragCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			fragCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			fragCreateInfo.module = fragShaderModule;
			fragCreateInfo.pName = "main";

			VkPipelineShaderStageCreateInfo shaderStages[] = {
				vertCreateInfo, fragCreateInfo
			};

			// todo: vertices are hardcoded in the shader for now
			VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
			vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			vertexInputInfo.vertexBindingDescriptionCount = 0;
			vertexInputInfo.pVertexBindingDescriptions = NULL;
			vertexInputInfo.vertexAttributeDescriptionCount = 0;
			vertexInputInfo.pVertexAttributeDescriptions = NULL;

			VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
			inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			inputAssembly.primitiveRestartEnable = VK_FALSE;


			VkViewport viewport = {};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = (float)swapChainExtent.width;
			viewport.height = (float)swapChainExtent.height;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;

			VkRect2D scissor = {};
			scissor.offset = { 0,0 };
			scissor.extent = swapChainExtent;

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
			rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
			rasterizer.depthBiasEnable = VK_FALSE;
			rasterizer.depthBiasConstantFactor = 0.0f;
			rasterizer.depthBiasClamp = 0.0f;
			rasterizer.depthBiasSlopeFactor = 0.0f;

			VkPipelineMultisampleStateCreateInfo multisampling = {};
			multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			multisampling.sampleShadingEnable = VK_FALSE;
			multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
			multisampling.minSampleShading = 1.0f;
			multisampling.pSampleMask = NULL;
			multisampling.alphaToCoverageEnable = VK_FALSE;
			multisampling.alphaToOneEnable = VK_FALSE;

			VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
			colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
				| VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			colorBlendAttachment.blendEnable = VK_FALSE;
			colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
			colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
			colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
			colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

			VkPipelineColorBlendStateCreateInfo colorBlending = {};
			colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			colorBlending.logicOpEnable = VK_FALSE;
			colorBlending.logicOp = VK_LOGIC_OP_COPY;
			colorBlending.attachmentCount = 1;
			colorBlending.pAttachments = &colorBlendAttachment;
			colorBlending.blendConstants[0] = 0.0f; // Optional
			colorBlending.blendConstants[1] = 0.0f; // Optional
			colorBlending.blendConstants[2] = 0.0f; // Optional
			colorBlending.blendConstants[3] = 0.0f; // Optional

			// note: limited amount of state specifed above can be modified
			// with VKDynamicState

			// pipeline layout
			{
				VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
				pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
				pipelineLayoutInfo.setLayoutCount = 0; // Optional
				pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
				pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
				pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

				if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, NULL, &pipelineLayout) != VK_SUCCESS)
				{
					printf("failed to create pipeline layout \n");
					return;
				}
			}

			// render passes
			{
				VkAttachmentDescription colorAttachment = {};
				colorAttachment.format = swapChainImageFormat;
				colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
				colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

				// subpass
				VkSubpassDescription subpass = {};
				{
					VkAttachmentReference colorAttachmentRef = {};
					colorAttachmentRef.attachment = 0;
					colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

					subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
					subpass.colorAttachmentCount = 1;
					subpass.pColorAttachments = &colorAttachmentRef;
				}

				VkSubpassDependency dependency = {};
				dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
				dependency.dstSubpass = 0;
				dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				dependency.srcAccessMask = 0;
				dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

				VkRenderPassCreateInfo renderPassInfo = {};
				renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
				renderPassInfo.attachmentCount = 1;
				renderPassInfo.pAttachments = &colorAttachment;
				renderPassInfo.subpassCount = 1;
				renderPassInfo.pSubpasses = &subpass;
				renderPassInfo.dependencyCount = 1;
				renderPassInfo.pDependencies = &dependency;

				if (vkCreateRenderPass(device, &renderPassInfo, NULL, &renderPass) != VK_SUCCESS)
				{
					printf("failed to create render pass\n");
					return;
				}
			}

			// finally, create pipeline object
			{
				VkGraphicsPipelineCreateInfo pipelineInfo = {};
				pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
				pipelineInfo.stageCount = 2;
				pipelineInfo.pStages = shaderStages;
				pipelineInfo.pVertexInputState = &vertexInputInfo;
				pipelineInfo.pInputAssemblyState = &inputAssembly;
				pipelineInfo.pViewportState = &viewportState;
				pipelineInfo.pRasterizationState = &rasterizer;
				pipelineInfo.pMultisampleState = &multisampling;
				pipelineInfo.pDepthStencilState = NULL;
				pipelineInfo.pColorBlendState = &colorBlending;
				pipelineInfo.pDynamicState = NULL;
				pipelineInfo.layout = pipelineLayout;
				pipelineInfo.renderPass = renderPass;
				pipelineInfo.subpass = 0;
				pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
				pipelineInfo.basePipelineIndex = -1;

				if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &graphicsPipeline) != VK_SUCCESS)
				{
					printf("failed to create graphics pipeline\n");
					return;
				}
			}
		}

		// frame buffer
		{
			swapchainFrameBuffers.resize(swapChainImageViews.size());

			for (size_t i = 0; i < swapChainImageViews.size(); i++)
			{
				VkImageView attachments[] = {
					swapChainImageViews[i]
				};

				VkFramebufferCreateInfo framebufferInfo = {};
				framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
				framebufferInfo.renderPass = renderPass;
				framebufferInfo.attachmentCount = 1;
				framebufferInfo.pAttachments = attachments;
				framebufferInfo.width = swapChainExtent.width;
				framebufferInfo.height = swapChainExtent.height;
				framebufferInfo.layers = 1;

				if (vkCreateFramebuffer(device, &framebufferInfo, NULL, &swapchainFrameBuffers[i]) != VK_SUCCESS)
				{
					printf("failed to create framebuffer\n");
					return;
				}
			}
		}

		// command buffer
		{
			// pool
			{
				VkCommandPoolCreateInfo poolInfo = {};
				poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
				poolInfo.queueFamilyIndex = queueIndicies.graphicsFamily;
				poolInfo.flags = 0;

				if (vkCreateCommandPool(device, &poolInfo, NULL, &commandPool) != VK_SUCCESS)
				{
					printf("failed to create command buffer\n");
					return;
				}
			}

			// buffers
			{
				commandBuffers.resize(swapchainFrameBuffers.size());

				VkCommandBufferAllocateInfo allocInfo = {};
				allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
				allocInfo.commandPool = commandPool;
				allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
				allocInfo.commandBufferCount = (unsigned int)commandBuffers.size();

				if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
				{
					printf("failed to allocate command buffer\n");
					return;
				}

				for (size_t i = 0; i < commandBuffers.size(); i++)
				{
					VkCommandBufferBeginInfo beginInfo = {};
					beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
					beginInfo.flags = 0;
					beginInfo.pInheritanceInfo = NULL;

					if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS)
					{
						printf("failed to begin record command buffer\n");
						return;
					}

					// starting render pass
					{
						VkRenderPassBeginInfo renderPassInfo = {};
						renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
						renderPassInfo.renderPass = renderPass;
						renderPassInfo.framebuffer = swapchainFrameBuffers[i];
						renderPassInfo.renderArea.offset = { 0,0 };
						renderPassInfo.renderArea.extent = swapChainExtent;

						VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
						renderPassInfo.clearValueCount = 1;
						renderPassInfo.pClearValues = &clearColor;

						vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
					}

					vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

					vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);

					vkCmdEndRenderPass(commandBuffers[i]);

					if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
					{
						printf("failed to record command buffer\n");
						return;
					}
				}
			}
		}

		// create semaphores
		{
			imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
			renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
			inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
			imagesInFlight.resize(swapChainImages.size(), VK_NULL_HANDLE);

			VkSemaphoreCreateInfo semaphoreInfo = {};
			semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

			VkFenceCreateInfo fenceInfo = {};
			fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

			for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
			{
				if (vkCreateSemaphore(device, &semaphoreInfo, NULL, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
					vkCreateSemaphore(device, &semaphoreInfo, NULL, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
					vkCreateFence(device, &fenceInfo, NULL, &inFlightFences[i]) != VK_SUCCESS)
				{
					printf("failed to create semaphores");
					return;
				}
			}
		}

		bInited = true;
	}

	void render()
	{
		if (!bInited)  return;

		vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

		unsigned int imageIndex = 0;
		vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

		// check if previous frame is using this image
		if (imagesInFlight[imageIndex] != VK_NULL_HANDLE)
		{
			vkWaitForFences(device, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
		}
		// mark the image as now being used by this frame
		imagesInFlight[imageIndex] = inFlightFences[currentFrame];

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

		VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		vkResetFences(device, 1, &inFlightFences[currentFrame]);

		if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
		{
			printf("failed to submit command\n");
			return;
		}

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { swapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.pResults = NULL;

		vkQueuePresentKHR(presentQueue, &presentInfo);

		currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	void cleanup()
	{
		if (enableValidationLayers)
		{
			destroyDebugUtilsMessengerEXT(instance, debugMessenger, NULL);
		}

		for (auto framebuffer : swapchainFrameBuffers)
		{
			vkDestroyFramebuffer(device, framebuffer, NULL);
		}

		for (auto imageView : swapChainImageViews)
		{
			vkDestroyImageView(device, imageView, NULL);
		}

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(device, renderFinishedSemaphores[i], NULL);
			vkDestroySemaphore(device, imageAvailableSemaphores[i], NULL);
			vkDestroyFence(device, inFlightFences[i], NULL);
		}

		vkDestroyCommandPool(device, commandPool, NULL);
		vkDestroyPipeline(device, graphicsPipeline, NULL);
		vkDestroyPipelineLayout(device, pipelineLayout, NULL);
		vkDestroyRenderPass(device, renderPass, NULL);
		vkDestroySwapchainKHR(device, swapChain, NULL);
		vkDestroySurfaceKHR(instance, surface, NULL);
		vkDestroyInstance(instance, NULL);
		vkDestroyDevice(device, NULL);
		vkDestroyShaderModule(device, fragShaderModule, NULL);
		vkDestroyShaderModule(device, vertShaderModule, NULL);
	}
}
