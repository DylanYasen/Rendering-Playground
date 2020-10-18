#include "Math.h"
class Camera
{
public:
    Camera(float fov, float aspectRatio, float near, float far,
           const vec3 &eye, const vec3 &lookat);
    ~Camera();

    // todo: abstract this out to some interface
    void DrawDebugMenu();

    mat4 projMatrix;
    mat4 viewMatrix;

    float fov;
    float aspect;

    float near;
    float far;

    vec3 eyePos;
    vec3 lookatPos;

    vec3 up;
    vec3 right;
    vec3 forward;

private:
    // todo: do this in transform
    void UpdateVectors();
};
