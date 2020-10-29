#include "Quad.h"

#include "IndexBuffer.h"
#include "VertexArray.h"
#include "Shader.h"
#include "Renderer.h"

static const float vertices[] = {
    -1, -1, 0.0, 0.0, 0.0,
    -1, 1, 0.0, 0.0, 1.0,
    1, 1, 0.0, 1.0, 1.0,
    1, -1, 0.0, 1.0, 0.0};

static const unsigned int indices[] = {
    0, 1, 2,
    0, 2, 3};

Quad::Quad()
{
    m_vao = new VertexArray();
    {
        m_vbo = new VertexBuffer(vertices, sizeof(vertices));

        VertexBufferLayout layout;
        layout.Push<float>(3); // pos
        layout.Push<float>(2); // tex coords
        m_vao->AddBuffer(*m_vbo, layout);

        m_ibo = new IndexBuffer(indices, 6);
    }
}

Quad::~Quad()
{
}

void Quad::PreRender(const Scene *scene, const Renderer *renderer, Shader *shader)
{
}

void Quad::Render(const Scene *scene, const Renderer *renderer, Shader *shader)
{
    shader->SetUniformMat4f("u_m", m_transform.GetWorld());
    renderer->Render(*m_vao, *m_ibo, *shader);
}

void Quad::Destroy()
{
}
