#pragma once
#include "HandmadeMath.h"

#include <assimp/vector3.h>
#include <assimp/matrix4x4.h>

typedef hmm_m4 mat4;
typedef hmm_v2 vec2;
typedef hmm_v3 vec3;
typedef hmm_quaternion quat;

namespace Math
{
    static mat4 aiMat4toMat4(const aiMatrix4x4 &aimat4)
    {
        mat4 result;

        result.Elements[0][0] = aimat4.a1;
        result.Elements[0][1] = aimat4.b1;
        result.Elements[0][2] = aimat4.c1;
        result.Elements[0][3] = aimat4.d1;

        result.Elements[1][0] = aimat4.a2;
        result.Elements[1][1] = aimat4.b2;
        result.Elements[1][2] = aimat4.c2;
        result.Elements[1][3] = aimat4.d2;

        result.Elements[2][0] = aimat4.a3;
        result.Elements[2][1] = aimat4.b3;
        result.Elements[2][2] = aimat4.c3;
        result.Elements[2][3] = aimat4.d3;

        result.Elements[3][0] = aimat4.a4;
        result.Elements[3][1] = aimat4.b4;
        result.Elements[3][2] = aimat4.c4;
        result.Elements[3][3] = aimat4.d4;

        return result;
    }

    static vec3 aiVec3ToVec3(const aiVector3D &aivec3)
    {
        return HMM_Vec3(aivec3.x, aivec3.y, aivec3.z);
    }
}; 