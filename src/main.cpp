#define SDL_MAIN_HANDLED
#include "SDL.h"

#include <stdio.h>
#include <cassert>

//#include "vulkan.h"
//#include "d3d.h"
#include "gl.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

const int WIDTH = 800;
const int HEIGHT = 600;

unsigned char* loadTexture(const char* filepath)
{
	int x, y, n;
	unsigned char* data = stbi_load(filepath, &x, &y, &n, 0);

	return data;
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
			SDL_WINDOW_SHOWN |SDL_WINDOW_OPENGL /*| SDL_WINDOW_VULKAN*/);


		loadTexture("resources/textures/mob.png");

		if (window == NULL)
		{
			printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
		}
		else
		{
#ifdef WIN32
			//DX11::initContext(GetActiveWindow());
			//Vulkan::initVulkan(window);
#else
#endif
			GL::init(window);

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
				/*Vulkan::render();*/
#else
				GL::render();
#endif
				GL::render();
			}
		}

		SDL_DestroyWindow(window);
		SDL_Quit();

		return 0;
	}
}