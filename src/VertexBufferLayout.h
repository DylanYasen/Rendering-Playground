#pragma once

#include <vector>

// todo: reduce dependency
#include "gl.h"

struct VertexBufferElement
{
	unsigned int type;  // gltype
	unsigned int count;
	unsigned char normalized;

	VertexBufferElement(unsigned int t, unsigned int c, unsigned char n)
		:type(t),
		count(c),
		normalized(n)
	{
	}

	static unsigned int GetSizeOfType(unsigned int type)
	{
		switch (type)
		{
			case GL_FLOAT: return sizeof(GLfloat);
			case GL_UNSIGNED_INT: return sizeof(GLuint);
			case GL_UNSIGNED_BYTE: return sizeof(GLubyte);
		default:
			ASSERT(false);
			return 0;
		}
	}
};

class VertexBufferLayout
{
private:
	std::vector<VertexBufferElement> elements;
	unsigned int stride;

public:
	VertexBufferLayout()
		:stride(0)
	{
	}

	template<typename T>
	void Push(unsigned int count)
	{
		static_assert(false);
	}

	template<>
	void Push<float>(unsigned int count)
	{
		elements.emplace_back(GL_FLOAT, count, GL_FALSE);
		stride += VertexBufferElement::GetSizeOfType(GL_FLOAT) * count;
	}

	template<>
	void Push<unsigned int>(unsigned int count)
	{
		elements.emplace_back(GL_UNSIGNED_INT, count, GL_FALSE);
		stride += VertexBufferElement::GetSizeOfType(GL_UNSIGNED_INT) * count;
	}

	template<>
	void Push<unsigned char>(unsigned int count)
	{
		elements.emplace_back(GL_UNSIGNED_BYTE, count, GL_TRUE);
		stride += VertexBufferElement::GetSizeOfType(GL_UNSIGNED_BYTE) * count;
	}

	inline const std::vector<VertexBufferElement>& GetElements() const { return elements; }
	inline unsigned int GetStride() const { return stride; }
};