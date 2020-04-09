#include <stdio.h>
#include <fstream>
#include <iostream>
#include <string>
#include <malloc.h>
#include <sstream>

#include "gl.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "VertexBufferLayout.h"
#include "Shader.h"
#include "Renderer.h"

void glClearError()
{
	while (glGetError() != GL_NO_ERROR);
}

bool glCheckError(const char* func, const char* file, int line)
{
	while (GLenum error = glGetError())
	{
		printf("[OpenGL Error] code:[%d] file:[%s] func:[%s] line:[%d]\n", error, file, func, line);
		return false;
	}
	return true;
}

namespace GL
{
	VertexBuffer vb;
	IndexBuffer ib;
	VertexArray* va;
	Shader* shader;
	Renderer* renderer;

	bool bInit = false;
	SDL_Window* window = NULL;
	int location = -1;

	namespace utils
	{
		void printShaderLog(unsigned int shader)
		{
			if (glIsShader(shader))
			{
				int infoLogLength = 0;
				int maxLength = infoLogLength;

				GLCall(glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength));

				char* infoLog = (char*)_malloca(maxLength * sizeof(char));
				GLCall(glGetShaderInfoLog(shader, maxLength, &infoLogLength, infoLog));
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
	}

	namespace triangle
	{
		bool init()
		{
			shader = new Shader("resources/shaders/basic.shader");

			GLCall(glClearColor(0.f, 0.f, 0.f, 1.f));

			const float vertexData[] =
			{
				-0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
				 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
				 0.0f,  0.5f, 0.0f, 0.0f, 1.0f
			};

			va = new VertexArray();
			{
				vb = VertexBuffer(vertexData, 5 * 3 * sizeof(float));

				VertexBufferLayout layout;
				layout.Push<float>(2); // pos
				layout.Push<float>(3); // color

				va->AddBuffer(vb, layout);
			}

			const unsigned int indexData[] = { 0,1,2 };
			ib = IndexBuffer(indexData, 3);

			return true;
		}

		float r = 0.0f;
		float increment = 0.05f;

		void draw()
		{
			renderer->Clear();

			shader->Bind();
			shader->SetUniform4f("u_color", r, 0.1f, 0.1f, 1.0f);

			renderer->Draw(*va, ib, *shader);

			if (r > 1.0f)
			{
				increment = -0.05f;
			}
			else if (r < 0.0f)
			{
				increment = 0.05f;
			}
			r += increment;
		}
	}

	namespace rect
	{
		bool init()
		{
			GLCall(glClearColor(0.f, 0.f, 0.f, 1.f));

			const float vertexData[] =
			{
				-0.5f, -0.5f,
				 0.5, -0.5f,
				 0.5f, 0.5f,

				 //0.5f, 0.5f,
				-0.5f, 0.5f,
				//-0.5f, -0.5f,
			};

			va = new VertexArray();
			{
				vb = VertexBuffer(vertexData, sizeof(vertexData));

				VertexBufferLayout layout;
				layout.Push<float>(2);

				va->AddBuffer(vb, layout);
			}
			
			const unsigned int indexData[] = { 0,1,2,2,3,0 };
			ib = IndexBuffer(indexData, 6);
				
			//GLCall(glGenBuffers(1, &VBO));
			//GLCall(glBindBuffer(GL_ARRAY_BUFFER, VBO));
			//GLCall(glBufferData(GL_ARRAY_BUFFER, 4 * 3 * sizeof(float), vertexData, GL_STATIC_DRAW));

			//const unsigned int indexData[] = { 0,1,2,2,3,0 };
			//GLCall(glGenBuffers(1, &IBO));
			//GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO));
			//GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indexData, GL_STATIC_DRAW));

			//GLCall(glEnableVertexAttribArray(0));
			//GLCall(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0));

			return true;
		}

		void draw()
		{
			GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0));
		}
	}

	bool initGL()
	{
		renderer = new Renderer();
		
		return triangle::init();
		//return rect::init();
	}

	void init(SDL_Window* win)
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


		triangle::draw();
		//rect::draw();

		SDL_GL_SwapWindow(window);
	}
}