#define SDL_MAIN_HANDLED
#include "SDL.h"
#include <stdio.h>

#include <glad/glad.h>

#ifdef WIN32
#include <wtypes.h>
#include <d3dcommon.h>
#include <d3d11.h>
#include <Windows.h>
#include <wrl/client.h>
#include <dxgi1_3.h>
#include <d3dcompiler.h>
#include <tchar.h>
#include <cassert>
#endif 

#include "vulkan/vulkan_core.h"
#include "SDL_vulkan.h"
#include <vector>
#ifdef WIN32
#include "vulkan/vulkan_win32.h"
#endif 

#include <unordered_set>
#include <fstream>

const int WIDTH = 800;
const int HEIGHT = 600;

namespace GL
{
	unsigned int programID = 0;
	int posAttribute = 0;
	int colorAttribute = 0;
	unsigned int IBO = 0;
	unsigned int VBO = 0;
	bool bInit = false;
	SDL_Window* window = NULL;

	namespace utils
	{
		void printProgramLog(unsigned int program)
		{
			if (glIsProgram(program))
			{
				int infoLogLength = 0;
				int maxLength = infoLogLength;

				glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

				char* infoLog = (char*)_malloca(maxLength * sizeof(char));
				glGetProgramInfoLog(program, maxLength, &infoLogLength, infoLog);
				if (infoLogLength > 0)
				{
					printf("%s\n", infoLog);
				}
			}
			else
			{
				printf("Name %d is not a program\n", program);
			}
		}

		void printShaderLog(unsigned int shader)
		{
			if (glIsShader(shader))
			{
				int infoLogLength = 0;
				int maxLength = infoLogLength;

				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

				char* infoLog = (char*)_malloca(maxLength * sizeof(char));
				glGetShaderInfoLog(shader, maxLength, &infoLogLength, infoLog);
				if (infoLogLength > 0)
				{
					printf("%s\n", infoLog);
				}
			}
			else
			{
				printf("Name %d is not a shader\n", shader);
			}
		}

		unsigned int compileShader(const char* shaderSrc[], unsigned int type)
		{
			unsigned int shader = glCreateShader(type);
			glShaderSource(shader, 1, shaderSrc, NULL);
			glCompileShader(shader);

			int compiled = GL_FALSE;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
			if (compiled != GL_TRUE)
			{
				printf("Unable to compile vertex shader %d!\n", shader);
				utils::printShaderLog(shader);
				return NULL;
			}
			else
			{
				return shader;
			}
		}
	}

	bool initGL()
	{
		programID = glCreateProgram();

		const char* vtxSrc[] =
		{
			"#version 330 core\n"
			"in vec2 position;\n"
			"in vec3 color;\n"
			"out vec3 Color;\n"
			"void main()\n"
			"{\n"
				"gl_Position = vec4( position, 0.0, 1.0 );\n"
				"Color = color;\n"
			"}\n"
		};

		unsigned int vertexShader = utils::compileShader(vtxSrc, GL_VERTEX_SHADER);
		if (vertexShader == NULL) return false;

		const char* fragSrc[] =
		{
			"#version 330 core\n"
			"in vec3 Color;\n"
			"out vec4 outColor;\n"
			"void main()\n"
			"{\n"
				"outColor = vec4(Color, 1.0);\n"
			"}\n"
		};
		unsigned int fragShader = utils::compileShader(fragSrc, GL_FRAGMENT_SHADER);
		if (fragShader == NULL) return false;
		glBindFragDataLocation(fragShader, 0, "outColor");

		glAttachShader(programID, vertexShader);
		glAttachShader(programID, fragShader);

		glLinkProgram(programID);
		glValidateProgram(programID);

		int programSuccess = GL_TRUE;
		glGetProgramiv(programID, GL_LINK_STATUS, &programSuccess);
		if (programSuccess != GL_TRUE)
		{
			printf("Error linking program %d\n", programID);
			return false;
		}
		else
		{
			glClearColor(0.f, 0.f, 0.f, 1.f);

			const float vertexData[] =
			{
				-0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
				 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
				 0.0f,  0.5f, 0.0f, 0.0f, 1.0f
			};
			glGenBuffers(1, &VBO);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, 5 * 3 * sizeof(float), vertexData, GL_STATIC_DRAW);

			const unsigned int indexData[] = { 0,1,2 };
			glGenBuffers(1, &IBO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * sizeof(unsigned int), indexData, GL_STATIC_DRAW);

			glUseProgram(programID);
			posAttribute = glGetAttribLocation(programID, "position");
			glEnableVertexAttribArray(posAttribute);
			glVertexAttribPointer(posAttribute, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
			colorAttribute = glGetAttribLocation(programID, "color");
			glEnableVertexAttribArray(colorAttribute);
			glVertexAttribPointer(colorAttribute, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
		}

		return true;
	}

	void initContext(SDL_Window* win)
	{
		if (!win) return;
		window = win;

		SDL_GLContext context = SDL_GL_CreateContext(window);
		if (context == NULL)
		{
			printf("OpenGL context could not be created! SDL Error: %s\n", SDL_GetError());
			return;
		}

		if (!gladLoadGL())
		{
			printf("gladLoadGL failed");
			return;
		}

		printf("Vendor:   %s\n", glGetString(GL_VENDOR));
		printf("Renderer: %s\n", glGetString(GL_RENDERER));
		printf("Version:  %s\n", glGetString(GL_VERSION));

		// Vsync
		if (SDL_GL_SetSwapInterval(1) < 0)
		{
			printf("Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());
		}

		if (!initGL())
		{
			printf("Unable to initialize OpenGL!\n");
		}
		else
		{
			bInit = true;
		}
	}

	void render()
	{
		if (!bInit) return;

		glClear(GL_COLOR_BUFFER_BIT);

		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);

		SDL_GL_SwapWindow(window);
	}
}

#ifdef WIN32
namespace DX11
{
	struct vertex
	{
		float x, y;
		float r, g, b;
	};

	namespace utils
	{
		void ErrorDescription(HRESULT hr)
		{
			if (FACILITY_WINDOWS == HRESULT_FACILITY(hr))
				hr = HRESULT_CODE(hr);
			TCHAR* szErrMsg;

			if (FormatMessage(
				FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
				NULL, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPTSTR)&szErrMsg, 0, NULL) != 0)
			{
				_tprintf(TEXT("%s"), szErrMsg);
				LocalFree(szErrMsg);
			}
			else
				_tprintf(TEXT("[Could not find a description for error # %#x.]\n"), hr);
		}

		ID3DBlob* compileShader(ID3D11Device* device, LPCWSTR file, LPCSTR profile, LPCSTR entrypoint)
		{
			if (!device || !file || !profile) return 0;

			UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
			flags |= D3DCOMPILE_DEBUG;
#endif
			//LPCSTR profile = (device->GetFeatureLevel() >= D3D_FEATURE_LEVEL_11_0) ? "cs_5_0" : "cs_4_0";
			const D3D_SHADER_MACRO defines[] = { 0 };
			ID3DBlob* shaderBlob = 0;
			ID3DBlob* errorBlob = 0;

			HRESULT hr = D3DCompileFromFile(file, defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
				entrypoint, profile, flags, 0, &shaderBlob, &errorBlob);

			if (FAILED(hr))
			{
				ErrorDescription(hr);
				if (errorBlob) OutputDebugStringA((char*)errorBlob->GetBufferPointer());
				if (shaderBlob) shaderBlob->Release();
				return 0;
			}
			else
			{
				return shaderBlob;
			}
		}
	}

	static const D3D_FEATURE_LEVEL levels[] = {
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};

	const float backgroundColor[4] = { 0.0,0.0,0.0,1.0f };

	HWND window;
	D3D_FEATURE_LEVEL featureLevel;
	Microsoft::WRL::ComPtr<ID3D11Device> device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
	Microsoft::WRL::ComPtr<IDXGISwapChain> swapchain;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView;
	D3D11_TEXTURE2D_DESC backbufferDesc;
	D3D11_VIEWPORT viewport;

	ID3D11InputLayout* inputLayout = 0;
	ID3D11Buffer* vertexBuffer = 0;
	unsigned int vStride = sizeof(vertex);
	unsigned int vOffset = 0;
	unsigned int vCount = 3;
	ID3D11Buffer* indexBuffer = 0;

	ID3D11PixelShader* ps = 0;
	ID3D11VertexShader* vs = 0;

	bool bInited = false;

	void createSwapChain()
	{
		DXGI_SWAP_CHAIN_DESC desc;
		ZeroMemory(&desc, sizeof(DXGI_SWAP_CHAIN_DESC));
		desc.Windowed = TRUE;
		desc.BufferCount = 2;
		desc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
		desc.OutputWindow = window;

		Microsoft::WRL::ComPtr<IDXGIDevice3> dxgiDevice;
		device.As(&dxgiDevice);

		Microsoft::WRL::ComPtr<IDXGIAdapter> adapter;
		Microsoft::WRL::ComPtr<IDXGIFactory> factory;

		HRESULT hr = dxgiDevice->GetAdapter(&adapter);
		if (SUCCEEDED(hr))
		{
			adapter->GetParent(IID_PPV_ARGS(&factory));
			hr = factory->CreateSwapChain(device.Get(), &desc, &swapchain);
		}
	}

	void createRenderTarget()
	{
		Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
		HRESULT hr = swapchain->GetBuffer(
			0,
			__uuidof(ID3D11Texture2D),
			(void**)backBuffer.GetAddressOf()
		);

		hr = device->CreateRenderTargetView(
			backBuffer.Get(),
			0,
			renderTargetView.GetAddressOf()
		);

		backBuffer->GetDesc(&backbufferDesc);
	}

	void createViewPort()
	{
		ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
		viewport.Height = (float)backbufferDesc.Height;
		viewport.Width = (float)backbufferDesc.Width;
		viewport.MinDepth = 0;
		viewport.MaxDepth = 1;

		context->RSSetViewports(1, &viewport);
	}

	void initContext(HWND win)
	{
		window = win;
		unsigned int deviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#if defined(DEBUG) || defined(_DEBUG)
		deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		HRESULT hr = D3D11CreateDevice(
			nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			0,
			deviceFlags,
			levels,
			ARRAYSIZE(levels),
			D3D11_SDK_VERSION,
			&device,
			&featureLevel,
			&context
		);

		if (FAILED(hr))
		{
			printf("failed to create d3d11 device");
			return;
		}
		else
		{
			createSwapChain();
			// todo:depth buffer
			createRenderTarget();
			createViewPort();

#if 0
			ID3D10Blob* psblob = utils::compileShader(device.Get(), L"shaders/pixel.hlsl", "ps_4_0_level_9_1", "ps_main");
			ID3D10Blob* vsblob = utils::compileShader(device.Get(), L"shaders/vertex.hlsl", "vs_4_0_level_9_1", "vs_main");
#endif

			ID3D10Blob* psblob = 0;
			D3DReadFileToBlob(L"pixel.cso", &psblob);

			ID3D10Blob* vsblob = 0;
			D3DReadFileToBlob(L"vertex.cso", &vsblob);

			hr = device->CreatePixelShader(psblob->GetBufferPointer(), psblob->GetBufferSize(), 0, &ps);
			assert(SUCCEEDED(hr));

			hr = device->CreateVertexShader(vsblob->GetBufferPointer(), vsblob->GetBufferSize(), 0, &vs);
			assert(SUCCEEDED(hr));

			context->PSSetShader(ps, 0, 0);
			context->VSSetShader(vs, 0, 0);

			D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
			{
				{"POS", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
				{"COL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
			};

			hr = device->CreateInputLayout(
				inputElementDesc,
				ARRAYSIZE(inputElementDesc),
				vsblob->GetBufferPointer(),
				vsblob->GetBufferSize(),
				&inputLayout
			);
			assert(SUCCEEDED(hr));

			// vdata
			const vertex vertextData[] =
			{
				{-0.5f, -0.5f, 1.0f, 0.0f, 0.0f},
				{ 0.0f,  0.5f, 0.0f, 0.0f, 1.0f},
				{ 0.5f, -0.5f, 0.0f, 1.0f, 0.0f}
			};

			// vbuffer
			{
				D3D11_BUFFER_DESC buffdesc = { 0 };
				buffdesc.ByteWidth = sizeof(vertextData);
				buffdesc.Usage = D3D11_USAGE_DEFAULT;
				buffdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
				buffdesc.StructureByteStride = sizeof(vertex);
				D3D11_SUBRESOURCE_DATA srdata = {};
				srdata.pSysMem = vertextData;
				HRESULT hr = device->CreateBuffer(
					&buffdesc,
					&srdata,
					&vertexBuffer
				);
				assert(SUCCEEDED(hr));
			}

			context->VSSetShader(vs, 0, 0);
			context->PSSetShader(ps, 0, 0);

			context->IASetInputLayout(inputLayout);
			context->IASetVertexBuffers(0, 1, &vertexBuffer, &vStride, &vOffset);

			context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			bInited = true;
		}
	}

	void render()
	{
		if (!bInited) return;

		context->ClearRenderTargetView(renderTargetView.Get(), backgroundColor);
		// todo: clear stencil after adding depth buffer

		context->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), 0);

		context->Draw(vCount, 0);

		swapchain->Present(1, 0);
	}
}
#endif // WIN32

namespace Vulkan
{
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
				
				createInfo.enabledExtensionCount =  ARRAYSIZE(deviceExtensions);
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

		VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
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

int main(int argc, char const* argv[])
{
	SDL_Window* window = NULL;
	SDL_Surface* screenSurface = NULL;

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
	}
	else
	{
		//Create window
		window = SDL_CreateWindow("Renderer Playground",
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			WIDTH,
			HEIGHT,
			SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN);

		if (window == NULL)
		{
			printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
		}
		else
		{
#ifdef WIN32
			//DX11::initContext(GetActiveWindow());
			Vulkan::initVulkan(window);
#else
			GL::initContext(window);
#endif

			bool bRunning = true;
			SDL_Event e;
			while (bRunning)
			{
				while (SDL_PollEvent(&e) != 0)
				{
					if (e.type == SDL_QUIT)
					{
						bRunning = false;
					}
				}

#ifdef WIN32
				//DX11::render();
				Vulkan::render();
#else
				GL::render();
#endif
			}
		}

		SDL_DestroyWindow(window);
		SDL_Quit();

		return 0;
	}
}