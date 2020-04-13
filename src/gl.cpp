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
#include "Texture.h"

#include "HandmadeMath.h"

const int WIDTH = 800;
const int HEIGHT = 600;

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
	Texture* texture;

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
				100.0f, 100.0f, 0.0f, 0.0f,
				200.0f, 100.0f, 1.0f, 0.0f,
				200.0f, 200.0f, 1.0f, 1.0f,
				100.0f, 200.0f, 0.0f, 1.0f
			};

			va = new VertexArray();
			{
				vb = VertexBuffer(vertexData, sizeof(vertexData));

				VertexBufferLayout layout;
				layout.Push<float>(2);
				layout.Push<float>(2);
				va->AddBuffer(vb, layout);
			}

			const unsigned int indexData[] = { 0,1,2,2,3,0 };
			ib = IndexBuffer(indexData, 6);

			hmm_mat4 proj = HMM_Orthographic(0.0f, WIDTH, 0, HEIGHT, -10, 10);
			hmm_mat4 view = HMM_Translate(HMM_Vec3(-100, 0, 0));
			hmm_mat4 model = HMM_Translate(HMM_Vec3(200, 200, 0));

			hmm_mat4 mvp = proj * view * model;

			shader = new Shader("resources/shaders/sprite.shader");
			shader->Bind();
			shader->SetUniformMat4f("u_mvp", mvp);

			texture = new Texture("resources/textures/mob.PNG");
			texture->Bind();
			shader->SetUniform1i("u_texture", 0);

			return true;
		}

		void draw()
		{
			renderer->Clear();
			renderer->Draw(*va, ib, *shader);
		}
	}

	namespace cube
	{
		hmm_mat4 model;
		hmm_mat4 view;
		hmm_mat4 projection;

		bool init()
		{
			model = HMM_Scale(HMM_Vec3(1, 1, 1)) * HMM_Rotate(30, HMM_Vec3(0, 1, 0)) * HMM_Translate(HMM_Vec3(0, 0, 0));
			view = HMM_Translate(HMM_Vec3(0.0f, 0.0f, -5.0f));
			projection = HMM_Perspective(60.0f, WIDTH / HEIGHT, 0.1f, 1000.0f);

			shader = new Shader("resources/shaders/cube.shader");
			shader->Bind();
			shader->SetUniformMat4f("u_m", model);
			shader->SetUniformMat4f("u_v", view);
			shader->SetUniformMat4f("u_p", projection);

			texture = new Texture("resources/textures/cube.PNG");
			texture->Bind();
			shader->SetUniform1i("u_tex", 0);

			const float vertices[] = {
				-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
				 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
				 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
				 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
				-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
				-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

				-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
				 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
				 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
				 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
				-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
				-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

				-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
				-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
				-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
				-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
				-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
				-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

				 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
				 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
				 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
				 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
				 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
				 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

				-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
				 0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
				 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
				 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
				-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
				-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

				-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
				 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
				 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
				 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
				-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
				-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
			};

			va = new VertexArray();
			{
				vb = VertexBuffer(vertices, sizeof(vertices));

				VertexBufferLayout layout;
				layout.Push<float>(3);
				layout.Push<float>(2);
				va->AddBuffer(vb, layout);
			}

			glEnable(GL_DEPTH_TEST);

			return true;
		}

		void draw()
		{
			renderer->Clear();

			va->Bind();
			shader->Bind();

			// rotate
			{
				model = model * HMM_Rotate(1.0f, HMM_Vec3(1, 1, 0));
				shader->SetUniformMat4f("u_m", model);
			}

			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
	}

	bool initGL()
	{
		renderer = new Renderer();

		GLCall(glEnable(GL_BLEND));
		GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

		//return triangle::init();
		//return rect::init();
		return cube::init();
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


		//triangle::draw();
		//rect::draw();
		cube::draw();

		SDL_GL_SwapWindow(window);
	}
}