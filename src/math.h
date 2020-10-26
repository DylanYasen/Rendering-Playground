#pragma once

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <assimp/vector3.h>
#include <assimp/matrix4x4.h>
#include <assimp/quaternion.h>

typedef glm::mat4 mat4;
typedef glm::vec4 vec4;
typedef glm::vec3 vec3;
typedef glm::vec2 vec2;
typedef glm::quat quat;

namespace Math
{
    static mat4 aiMat4toMat4(const aiMatrix4x4 &aimat4)
    {
        mat4 result;
        result[0][0] = aimat4.a1;
        result[0][1] = aimat4.b1;
        result[0][2] = aimat4.c1;
        result[0][3] = aimat4.d1;

        result[1][0] = aimat4.a2;
        result[1][1] = aimat4.b2;
        result[1][2] = aimat4.c2;
        result[1][3] = aimat4.d2;

        result[2][0] = aimat4.a3;
        result[2][1] = aimat4.b3;
        result[2][2] = aimat4.c3;
        result[2][3] = aimat4.d3;

        result[3][0] = aimat4.a4;
        result[3][1] = aimat4.b4;
        result[3][2] = aimat4.c4;
        result[3][3] = aimat4.d4;

        return result;
    }

    static vec3 aiVec3ToVec3(const aiVector3D &aivec3)
    {
        return glm::vec3(aivec3.x, aivec3.y, aivec3.z);
    }

    static quat aiQuatToQuat(const aiQuaternion &aiQuat)
    {
        return quat(aiQuat.w, aiQuat.x, aiQuat.y, aiQuat.z);
    }
}; // namespace Math
