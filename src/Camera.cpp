#include "Camera.h"
#include "Scene.h"

#include "imgui.h"

Camera::Camera(float fov, float aspectRatio, float near, float far,
               const vec3 &eye, const vec3 &lookat)
    : fov(fov), aspect(aspectRatio), near(near),
      far(far), eyePos(eye), lookatPos(lookat)
{
    projMatrix = glm::perspective(fov, aspect, near, far);

    UpdateVectors();
}

Camera::~Camera()
{
}

void Camera::UpdateVectors()
{
    const vec3 &worldup = vec3(0, 1, 0);

    forward = glm::normalize(lookatPos - eyePos);
    right = glm::normalize(glm::cross(worldup, forward));
    up = glm::normalize(glm::cross(right, forward));

    viewMatrix = glm::lookAt(eyePos, lookatPos, up);
}

bool Camera::DrawDebugMenu()
{
    if (ImGui::CollapsingHeader("camera"))
    {
        bool changed = ImGui::InputFloat3("eye pos", &eyePos.x);
        changed |= ImGui::InputFloat3("target pos", &lookatPos.x);

        if (changed)
        {
            UpdateVectors();
            return changed;
        }
    }

    return false;
}
