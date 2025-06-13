#pragma once
#include <glm/glm.hpp>

namespace GPU
{
    enum GPUShapeEnum
    {
        Sphere, Plane, Triangle, OBJ, BoundingBox
    };

#pragma pack(push, 1)
    struct GPUShapeData {
        // Bloc 1 : 16 bytes
        glm::vec3 color;        // 12 octets
        int type;               // 4 octets

        // Bloc 2 : 16 bytes
        int materialIndex;      // 4 octets
        float _pad1;            // 4 octets
        float _pad2;            // 4 octets
        float _pad3;            // 4 octets

        // Bloc 3 : 16 bytes
        glm::vec3 center;       // 12 octets
        float radius;           // 4 octets

        // Bloc 4 : 16 bytes
        glm::vec3 normal;       // 12 octets
        float dist;             // 4 octets

        // Total: 64 octets (4 blocs de 16)
    };
#pragma pack(pop)

    static_assert(sizeof(GPUShapeData) == 64, "Taille doit être 64 octets");
}




