#pragma once

#include <vector>
#include "Renderable.h"

class Quad : public Renderable
{
public:
    Quad();
    ~Quad();

    virtual void PreRender(const Scene *scene, const Renderer *renderer, Shader *shader) override;
    virtual void Render(const Scene *scene, const Renderer *renderer, Shader *shader) override;

    virtual void Destroy() override;
};