#pragma once

#include <glad/glad.h>
#include "SDL.h"

struct SDL_Window;

#define ASSERT(x) if(!(x)) __debugbreak();
#define GLCall(x) glClearError();\
	x;\
	ASSERT(glCheckError(#x, __FILE__, __LINE__))

void glClearError();
bool glCheckError(const char* func, const char* file, int line);

namespace GL
{
	void init(SDL_Window* win);
	void render();
}
