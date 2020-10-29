#pragma once
#include "Math.h"
#include "Transform.h"

class Light
{
public:
    Light();
    ~Light();

    bool DrawDebugMenu();

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    Transform transform;

    float near, far;
    float width, height;

    mat4 projectionMatrix;
    mat4 viewMatrix;
    mat4 lightSpaceMatrix;
};