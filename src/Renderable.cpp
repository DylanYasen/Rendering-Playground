#include "Renderable.h"

#include <vector>

#include "IndexBuffer.h"
#include "VertexArray.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"

Renderable::Renderable()
{
}

Renderable::Renderable(const std::vector<Vertex, Allocator<Vertex>> &v, const std::vector<unsigned int> &i)
    : m_vertices(v), m_indices(i)
{
}