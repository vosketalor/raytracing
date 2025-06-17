#pragma once
#include "glm/vec3.hpp"

namespace GPU
{
    struct GPUBVHNode {
        glm::vec3 minBounds;
        float pad1;
        glm::vec3 maxBounds;
        int leftChild;      // Index du fils gauche (-1 si feuille)
        int rightChild;     // Index du fils droit (-1 si feuille)
        int shapeIndex;     // Index de la forme si feuille (-1 sinon)
        int pad2;
        int pad3;
    };
    static_assert(sizeof(GPUBVHNode) % 16 == 0, "Doit être aligné sur 16 octets");
    static_assert(offsetof(GPUBVHNode, minBounds) % 16 == 0, "Vec3 doit être aligné sur 16 octets");
    static_assert(offsetof(GPUBVHNode, maxBounds) % 16 == 0, "Vec3 doit être aligné sur 16 octets");
}

