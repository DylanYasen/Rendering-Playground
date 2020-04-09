#pragma once

#include "VertexArray.h"
#include "Shader.h"
#include "IndexBuffer.h"

enum RendererAPI
{
	DX11,
	OPENGL,
	VULKAN
};

struct RendererInitInfo
{
	struct SDL_Window* window;
	int width;
	int height;
	RendererAPI api;
};

class Renderer
{
public:
	void Clear() const;
	void Draw(const VertexArray& va, const IndexBuffer& ib, const Shader& shader) const;
};
