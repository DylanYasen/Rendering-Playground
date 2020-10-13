#pragma once
#include "Math.h"

class Transform
{

public:
    Transform();
    mat4 GetLocal() const;
    mat4 GetWorld() const;

public:
    vec3 pos;
    vec3 scale;
    quat rot;
    mat4 parentTransform;
};