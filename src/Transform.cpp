#include "Transform.h"

Transform::Transform()
    : localToWorld(glm::identity<quat>()),
      position(1, 1, 1), scale(1, 1, 1),
      rotation(glm::identity<quat>())
{
}

Transform::Transform(vec3 pos)
    : position(pos), scale(1, 1, 1),
      rotation(glm::identity<quat>())
{
}

Transform::Transform(vec3 pos, vec3 scale, vec3 rot)
    : position(pos), scale(scale), rotation(rot)
{
}

mat4 Transform::GetLocal() const
{
    auto matrix = glm::identity<mat4>();
    {
        matrix = glm::translate(matrix, position);
        matrix = glm::scale(matrix, scale);
        matrix = glm::rotate(matrix, glm::angle(rotation), glm::axis(rotation));
    }
    return matrix;
}

mat4 Transform::GetWorld() const
{
    return localToWorld * GetLocal();
}
