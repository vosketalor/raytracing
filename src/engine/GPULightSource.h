#pragma once
#include "glm/vec3.hpp"

namespace GPU
{
    struct GPULightSource
    {
        glm::vec3 position;
        glm::vec3 colorDiffuse;
        glm::vec3 colorSpecular;
        float intensity;

        glm::vec3 uDir, vDir;
        float halfWidth, halfHeight;
    };
}
