#include "Light.h"

#include "imgui.h"

Light::Light()
    : ambient(HMM_Vec3(0.5, 0.5, 0.5)),
      diffuse(HMM_Vec3(1, 1, 1)),
      specular(HMM_Vec3(1, 1, 1))
{
}

Light::~Light()
{
}

void Light::DrawDebugMenu()
{
    if (ImGui::CollapsingHeader("light"))
    {
        ImGui::InputFloat3("position", transform.pos.Elements);
        ImGui::InputFloat3("ambient", ambient.Elements);
        ImGui::InputFloat3("diffuse", diffuse.Elements);
        ImGui::InputFloat3("specular", specular.Elements);
    }
}
