#pragma once

#include <vector>
#include "Renderable.h"

class Scene;

class Cube : public Renderable
{
public:
    Cube();

    virtual void PreRender(const Scene *scene, const Renderer *renderer, Shader* shader) override;
    virtual void Render(const Scene *scene, const Renderer *renderer, Shader *shader) override;

    virtual void Destroy() override;
};
