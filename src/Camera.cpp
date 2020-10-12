#include "Camera.h"
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"

Camera::Camera(float fov, float aspectRatio, float near, float far,
               const vec3 &eye, const vec3 &lookat)
    : m_fov(fov), m_aspect(aspectRatio), m_near(near), m_far(far),
      m_eye(eye), m_lookat(lookat)
{
    UpdateVectors();
}

Camera::~Camera()
{
}

void Camera::UpdateVectors()
{
    const vec3 &worldup = HMM_Vec3(0, 1, 0);

    m_forward = HMM_Normalize(m_lookat - m_eye);
    m_right = HMM_Normalize(HMM_Cross(m_forward, worldup));
    m_up = HMM_Normalize(HMM_Cross(m_right, m_forward));
}

mat4 Camera::GetViewMatrix() const
{
    return HMM_LookAt(m_eye, m_lookat, m_up);
}

mat4 Camera::GetProjectionMatrix() const
{
    return HMM_Perspective(m_fov, m_aspect, m_near, m_far);
}

void Camera::DrawDebugMenu()
{
    ImGui::InputFloat3("view pos", m_eye.Elements);
    UpdateVectors();
}
