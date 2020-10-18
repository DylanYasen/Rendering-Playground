#include "Light.h"

#include "imgui.h"

Light::Light()
    : ambient(vec3(0.5, 0.5, 0.5)),
      diffuse(vec3(1, 1, 1)),
      specular(vec3(1, 1, 1))
{
}

Light::~Light()
{
}

void Light::DrawDebugMenu()
{
    if (ImGui::CollapsingHeader("light"))
    {
        ImGui::InputFloat3("position", &transform.position.x);
        ImGui::InputFloat3("ambient", &ambient.x);
        ImGui::InputFloat3("diffuse", &diffuse.x);
        ImGui::InputFloat3("specular", &specular.x);
    }
}
