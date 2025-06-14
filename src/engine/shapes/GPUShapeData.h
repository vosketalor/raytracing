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
        glm::vec3 color=glm::vec3(0,0,0);        // 12 octets
        int type=0;               // 4 octets

        // Bloc 2 : 16 bytes
        int materialIndex;      // 4 octets
        float _pad1 = 0;            // 4 octets
        float _pad2 = 0;            // 4 octets
        float _pad3 = 0;            // 4 octets

        // Bloc 3 : 16 bytes
        glm::vec3 center=glm::vec3(0,0,0);       // 12 octets
        float radius=0;           // 4 octets

        // Bloc 4 : 16 bytes
        glm::vec3 normal=glm::vec3(0,0,0);       // 12 octets
        float dist=0;             // 4 octets

        // Total: 64 octets (4 blocs de 16)
    };
#pragma pack(pop)

    static_assert(sizeof(GPUShapeData) % 16 == 0, "Doit être aligné sur 16 octets");
    static_assert(offsetof(GPUShapeData, color) % 16 == 0, "Vec3 doit être aligné sur 16 octets");
    static_assert(offsetof(GPUShapeData, center) % 16 == 0, "Vec3 doit être aligné sur 16 octets");
    static_assert(offsetof(GPUShapeData, normal) % 16 == 0, "Vec3 doit être aligné sur 16 octets");
}




