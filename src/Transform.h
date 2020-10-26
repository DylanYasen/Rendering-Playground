#pragma once
#include "Math.h"

class Transform
{

public:
    Transform();
    Transform(vec3 pos);
    Transform(const mat4& transform);
    Transform(vec3 pos, vec3 scale, vec3 rot);

    mat4 GetLocal() const;
    mat4 GetWorld() const;

public:
    mat4 localToWorld;

    vec3 position;
    vec3 scale;
    quat rotation;

    vec3 up;
    vec3 right;
    vec3 forward;
};