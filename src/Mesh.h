#pragma once

#include <vector>
#include "Renderable.h"

class Scene;

class Mesh : public Renderable
{
public:
    Mesh();
    Mesh(const std::vector<Vertex> &v, const std::vector<unsigned int> &i,
         const std::vector<Texture *> &t, const Transform& trans, const std::string& name);

    ~Mesh();

    virtual void Render(const Scene* scene, const Renderer* renderer) override;
    virtual void Destroy() override;
};
