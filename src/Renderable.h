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

const int MAX_BONE_PER_VERT = 4;

struct Vertex
{
    vec3 position;
    vec3 normal;
    vec3 tangent;
    vec3 bitangent;
    vec2 texCoords;
    int boneIDs[MAX_BONE_PER_VERT] = {0};
    float weights[MAX_BONE_PER_VERT] = {0};
};

class Renderable
{
public:
    Renderable();
    Renderable(const std::vector<Vertex, Allocator<Vertex>> &v, const std::vector<unsigned int> &i);
    ~Renderable();

    virtual void PreRender(const Scene *scene, const Renderer *renderer, Shader *shader) = 0;
    virtual void Render(const Scene *scene, const Renderer *renderer, Shader *shader) = 0;

    virtual void Destroy() = 0;

public:
    Transform m_transform;

protected:
    std::vector<Vertex, Allocator<Vertex>> m_vertices;
    std::vector<unsigned int> m_indices;

    std::vector<Texture *> m_textures;

    VertexArray *m_vao;
    VertexBuffer *m_vbo;
    IndexBuffer *m_ibo;

    std::string m_name;
};
