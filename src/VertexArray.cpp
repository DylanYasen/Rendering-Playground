#include "VertexArray.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "gl.h"

VertexArray::VertexArray()
{
	GLCall(glGenVertexArrays(1, &rendererID));
}

VertexArray::~VertexArray()
{
	GLCall(glDeleteVertexArrays(1, &rendererID));
}

void VertexArray::AddBuffer(const VertexBuffer &vb, const VertexBufferLayout &layout)
{
	Bind();
	vb.Bind();

	const auto &elements = layout.GetElements();
	const unsigned int stride = layout.GetStride();
	unsigned int offset = 0;

	for (unsigned int i = 0; i < elements.size(); i++)
	{
		const auto &element = elements[i];
		GLCall(glEnableVertexAttribArray(i));
		if (element.type == GL_UNSIGNED_INT ||
			element.type == GL_INT)
		{
			GLCall(glVertexAttribIPointer(i, element.count, element.type, stride, (const void *)offset));
		}
		else
		{
			GLCall(glVertexAttribPointer(i, element.count, element.type, element.normalized, stride, (const void *)offset));
		}

		offset += element.count * VertexBufferElement::GetSizeOfType(element.type);
	}
}

void VertexArray::Bind() const
{
	GLCall(glBindVertexArray(rendererID));
}

void VertexArray::Unbind() const
{
	GLCall(glBindVertexArray(0));
}
