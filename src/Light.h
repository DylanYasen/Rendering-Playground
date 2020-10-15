#pragma once
#include "Math.h"
#include "Transform.h"

class Light
{
public:
    Light();
    ~Light();

    void DrawDebugMenu();

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    Transform transform;
};