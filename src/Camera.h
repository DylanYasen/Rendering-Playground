#include "Math.h"
class Camera
{
public:
    Camera(float fov, float aspectRatio, float near, float far,
           const vec3 &eye, const vec3 &lookat);
    ~Camera();

    mat4 GetProjectionMatrix() const;
    mat4 GetViewMatrix() const;

    vec3 GetEyePos() const { return m_eye; };

    // todo: abstract this out to some interface
    void DrawDebugMenu();

private:
    void UpdateVectors();

private:
    float m_fov;
    float m_aspect;
    
    float m_near;
    float m_far;

    vec3 m_eye;
    vec3 m_lookat;

    vec3 m_up;
    vec3 m_right;
    vec3 m_forward;
};
