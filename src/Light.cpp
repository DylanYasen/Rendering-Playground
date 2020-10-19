#include "Light.h"

#include "imgui.h"

Light::Light()
    : ambient(vec3(0.1)),
      diffuse(vec3(1)),
      specular(vec3(0.2))
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
        ImGui::ColorEdit3("ambient", &ambient.x);
        ImGui::ColorEdit3("diffuse", &diffuse.x);
        ImGui::ColorEdit3("specular", &specular.x);
    }
}
