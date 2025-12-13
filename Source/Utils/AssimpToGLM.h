#pragma once

#include <glm/glm.hpp>
#include <assimp/types.h>

static glm::mat4 AiToGlm(const aiMatrix4x4& m)
{
    glm::mat4 result;
    result[0][0] = m.a1; result[1][0] = m.a2; result[2][0] = m.a3; result[3][0] = m.a4;
    result[0][1] = m.b1; result[1][1] = m.b2; result[2][1] = m.b3; result[3][1] = m.b4;
    result[0][2] = m.c1; result[1][2] = m.c2; result[2][2] = m.c3; result[3][2] = m.c4;
    result[0][3] = m.d1; result[1][3] = m.d2; result[2][3] = m.d3; result[3][3] = m.d4;
    return result;
}

static glm::vec3 AiToGlm(const aiVector3D& v) {
    glm::vec3 result;
    result.x = v.x;
    result.y = v.y;
    result.z = v.z;
    return result;
}