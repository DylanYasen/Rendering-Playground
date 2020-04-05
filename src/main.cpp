#define SDL_MAIN_HANDLED
#include "SDL.h"
#include <stdio.h>

#include <glad/glad.h>

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

		glDrawElements(GL_TRIANGLES , 3, GL_UNSIGNED_INT, 0);

		SDL_GL_SwapWindow(window);
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
			800,
			600,
			SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

		if (window == NULL)
		{
			printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
		}
		else
		{
			GL::initContext(window);

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

				GL::render();
			}
		}

		SDL_DestroyWindow(window);
		SDL_Quit();

		return 0;
	}
}