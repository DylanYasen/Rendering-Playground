#include "Transform.h"
#include <glm/gtx/matrix_decompose.hpp>

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

Transform::Transform(const mat4 &transform)
{
    // glm::mat4
    vec3 skew;
    vec4 prespective;
    glm::decompose(transform, scale, rotation, position, skew, prespective);
}

mat4 Transform::GetLocal() const
{
    return glm::translate(mat4(1.0), position) * glm::mat4_cast(rotation) * glm::scale(mat4(1.0), scale);
}

mat4 Transform::GetWorld() const
{
    return localToWorld * GetLocal();
}
