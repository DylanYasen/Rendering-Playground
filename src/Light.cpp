#include "Light.h"

#include "imgui.h"
#include "Math.h"

#include <glm/gtc/matrix_transform.hpp>

Light::Light()
    : ambient(vec3(0.1)),
      diffuse(vec3(1)),
      specular(vec3(0.2)),
      width(80), height(80),
      near(1.0), far(1000),
      transform(vec3(0, 200, 100))
{
    projectionMatrix = glm::ortho(-width, width, height, -height, near, far);
    viewMatrix = glm::lookAt(transform.position, vec3(0, 0, 0), vec3(0, 0, 1));
    lightSpaceMatrix = projectionMatrix * viewMatrix;
}

Light::~Light()
{
}

bool Light::DrawDebugMenu()
{
    if (ImGui::CollapsingHeader("light"))
    {
        bool changed = ImGui::InputFloat3("position", &transform.position.x);
        changed |= ImGui::ColorEdit3("ambient", &ambient.x);
        changed |= ImGui::ColorEdit3("diffuse", &diffuse.x);
        changed |= ImGui::ColorEdit3("specular", &specular.x);

        if (changed)
        {
            viewMatrix = glm::lookAt(transform.position, vec3(0, 50, 0), vec3(0, 0, 1));
            lightSpaceMatrix = projectionMatrix * viewMatrix;
        }
        return changed;
    }

    return false;
}
