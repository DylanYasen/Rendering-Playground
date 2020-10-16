#pragma once

#include <glad/glad.h>
#include "SDL.h"
#include "FrameData.h"

struct SDL_Window;

#ifdef _MSC_VER
#define DEBUG_BREAK __debugbreak();
#else
#define DEBUG_BREAK printf("break\n"); // todo: other platforms
#endif


#define ASSERT(x) if(!(x)) DEBUG_BREAK;
#define GLCall(x) glClearError();\
	x;\
	ASSERT(glCheckError(#x, __FILE__, __LINE__))

void glClearError();
bool glCheckError(const char* func, const char* file, int line);

namespace GL
{
	void init(SDL_Window* win);
	void render(const FrameData& frameData);

    void processInput(const SDL_Event& e);
}
