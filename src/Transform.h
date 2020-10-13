#include "Math.h"

class Transform
{

public:
    Transform();
    const mat4 &GetLocal() const;
    const mat4 &GetWorld() const;

public:
    vec3 pos;
    vec3 scale;
    quat rot;
    mat4 parentTransform;
};