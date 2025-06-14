#pragma once
#include "glm/vec3.hpp"

namespace GPU
{
#pragma pack(push, 1)
    struct GPULightSource {
        glm::vec3 position=glm::vec3(0,0,0);      // 12 octets (0-11)
        float intensity=0;         // 4 octets  (12-15)
        glm::vec3 colorDiffuse=glm::vec3(0,0,0);  // 12 octets (16-27)
        float halfWidth=0;         // 4 octets  (28-31)
        glm::vec3 colorSpecular=glm::vec3(0,0,0); // 12 octets (32-43)
        float halfHeight=0;        // 4 octets  (44-47)
        glm::vec3 uDir=glm::vec3(0,0,0);          // 12 octets (48-59)
        float _pad1 = 0;             // 4 octets  (60-63)
        glm::vec3 vDir=glm::vec3(0,0,0);          // 12 octets (64-75)
        float _pad2 = 0;             // 4 octets  (76-79)
        // Total: 80 octets
    };
#pragma pack(pop)

    static_assert(sizeof(GPULightSource) % 16 == 0, "Doit être aligné sur 16 octets");
    static_assert(offsetof(GPULightSource, position) % 16 == 0, "Vec3 doit être aligné sur 16 octets");
    static_assert(offsetof(GPULightSource, colorDiffuse) % 16 == 0, "Vec3 doit être aligné sur 16 octets");
    static_assert(offsetof(GPULightSource, colorSpecular) % 16 == 0, "Vec3 doit être aligné sur 16 octets");
    static_assert(offsetof(GPULightSource, uDir) % 16 == 0, "Vec3 doit être aligné sur 16 octets");
    static_assert(offsetof(GPULightSource, vDir) % 16 == 0, "Vec3 doit être aligné sur 16 octets");
}