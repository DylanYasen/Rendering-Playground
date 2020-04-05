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
				{ 0.0f,  0.5f, 0.0f, 1.0f, 0.0f},
				{ 0.5f, -0.5f, 0.0f, 0.0f, 1.0f}
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
			800,
			600,
			SDL_WINDOW_OPENGL|SDL_WINDOW_SHOWN);

		if (window == NULL)
		{
			printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
		}
		else
		{
#ifdef WIN32
			DX11::initContext(GetActiveWindow());
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
				DX11::render();
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