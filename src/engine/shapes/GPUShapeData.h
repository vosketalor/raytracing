#pragma once
#include <glm/glm.hpp>

namespace GPU
{
    enum GPUShapeEnum
    {
        Sphere, Plane, Triangle, OBJ, BoundingBox
    };

    struct GPUShapeData {
        glm::vec3 color;
        int type;

        glm::vec3 center;
        float radius;
    };
}



