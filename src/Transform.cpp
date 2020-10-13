#include "Transform.h"

Transform::Transform()
    : pos(HMM_Vec3(0, 0, 0)), scale(HMM_Vec3(1, 1, 1)),
      rot(HMM_Quaternion(0, 0, 0, 0)),
      parentTransform(HMM_Mat4d(1))
{
}

mat4 Transform::GetLocal() const
{
    return HMM_Scale(scale) * HMM_QuaternionToMat4(rot) * HMM_Translate(pos);
}

mat4 Transform::GetWorld() const
{
    return parentTransform * GetLocal();
}
