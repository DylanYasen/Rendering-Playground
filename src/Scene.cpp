#include "Scene.h"
#include "Renderable.h"

#include "Camera.h"
#include "Light.h"
#include "imgui.h"

Scene::Scene(std::string name)
    : m_name(name)
{
}

Scene::~Scene()
{
    for (auto &r : m_renderables)
    {
        r->Destroy();
    }
}

void Scene::Render(Renderer *renderer) const
{
    for (auto &r : m_renderables)
    {
        r->Render(this, renderer);
    }

    ImGui::Begin("scene", 0, ImGuiWindowFlags_AlwaysAutoResize);
    m_camera->DrawDebugMenu();
    m_light->DrawDebugMenu();
    ImGui::End();
}