#include "Mesh.h"
#include "Texture.h"

#include "IndexBuffer.h"
#include "VertexArray.h"
#include "Shader.h"

#include "Renderer.h"
#include "Camera.h"
#include "Scene.h"

Mesh::Mesh()
{
}

// todo: investigate templated typedef alias issue
// template<typename T>
// using vector = std::vector<T, Allocator<T>>
Mesh::Mesh(const std::vector<Vertex, Allocator<Vertex>> &v, const std::vector<unsigned int> &i,
           const std::vector<Texture *> &t, const Transform &trans, const std::string &name)
    : Renderable(v, i)
{
    m_textures = t;
    m_name = name;
    m_transform = trans;

    m_vao = new VertexArray();
    {
        m_vbo = new VertexBuffer(&m_vertices[0], sizeof(Vertex) * m_vertices.size());

        VertexBufferLayout layout;
        layout.Push<float>(3); // pos
        layout.Push<float>(3); // normal
        layout.Push<float>(3); // tangent
        layout.Push<float>(3); // bitangent
        layout.Push<float>(2); // tex coords
        layout.Push<unsigned int>(4);
        layout.Push<float>(4);
        m_vao->AddBuffer(*m_vbo, layout);

        m_ibo = new IndexBuffer(&m_indices[0], m_indices.size());
    }
}

Mesh::~Mesh()
{
    delete m_vao, m_vbo, m_ibo;
}

void Mesh::PreRender(const Scene *scene, const Renderer *renderer)
{
}

void Mesh::Render(const Scene *scene, const Renderer *renderer)
{
    for (size_t i = 0; i < m_textures.size(); i++)
    {
        m_textures[i]->Bind(i);
        m_shader->SetUniform1i(m_textures[i]->GetMaterialTypeName(), i);
    }

    const auto &model = m_transform.GetWorld();
    const auto camera = scene->GetCamera();

    const mat4 &mvp =
        camera->projMatrix * camera->viewMatrix * model;

    // todo: move TBN calculation to cpu and clean up this uniform
    m_shader->SetUniformMat4f("u_m", model);
    m_shader->SetUniformMat4f("u_mvp", mvp);

    renderer->Render(*m_vao, *m_ibo, *m_shader);
}

void Mesh::Destroy()
{
}
