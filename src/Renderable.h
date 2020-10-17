#pragma once
#include "Transform.h"
#include "Allocator.h"

#include <vector>
#include <string>

class Texture;
class VertexArray;
class VertexBuffer;
class IndexBuffer;
class Shader;

class Renderer;
class Camera;
class Scene;

struct Vertex
{
    vec3 position;
    vec3 normal;
    vec3 tangent;
    vec3 bitangent;
    vec2 texCoords;
};

class Renderable
{
public:
    Renderable();
    Renderable(const std::vector<Vertex, Allocator<Vertex>> &v, const std::vector<unsigned int> &i);
    virtual void Render(const Scene* scene, const Renderer *renderer) = 0;
    virtual void Destroy() = 0;

    void SetShader(Shader *shader) { m_shader = shader; };

public:
    Transform m_transform;

protected:
    std::vector<Vertex, Allocator<Vertex>> m_vertices;
    std::vector<unsigned int> m_indices;

    std::vector<Texture *> m_textures;

    VertexArray *m_vao;
    VertexBuffer *m_vbo;
    IndexBuffer *m_ibo;
    Shader *m_shader;

    std::string m_name;
};
