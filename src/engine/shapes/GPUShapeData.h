#pragma once
#include <glm/glm.hpp>

namespace GPU
{
    enum GPUShapeEnum
    {
        Sphere, Plane, Triangle, OBJ
    };

#pragma pack(push, 1)
    struct GPUShapeData {
        glm::vec3 color=glm::vec3(0,0,0);        // 12 octets
        int type=0;               // 4 octets

        int materialIndex;      // 4 octets
        int wireframeEnabled; // 4 octets
        float pad1_=0;           // 4 octets
        int textureIndex=-1;            // 4 octets

        //Sphere
        glm::vec3 center=glm::vec3(0,0,0);       // 12 octets
        float radius=0;           // 4 octets

        //Plane
        glm::vec3 normal=glm::vec3(0,0,0);       // 12 octets
        float dist=0;             // 4 octets

        //Triangle
        glm::vec3 A = glm::vec3(0,0,0);
        float pad2_=0;

        glm::vec3 B = glm::vec3(0,0,0);
        float pad3_=0;

        glm::vec3 C = glm::vec3(0,0,0);
        float pad4_=0;
    };
#pragma pack(pop)
    static_assert(sizeof(GPUShapeData) % 16 == 0, "Doit être aligné sur 16 octets");
    static_assert(offsetof(GPUShapeData, color) % 16 == 0, "Vec3 doit être aligné sur 16 octets");
    static_assert(offsetof(GPUShapeData, center) % 16 == 0, "Vec3 doit être aligné sur 16 octets");
    static_assert(offsetof(GPUShapeData, normal) % 16 == 0, "Vec3 doit être aligné sur 16 octets");

}




