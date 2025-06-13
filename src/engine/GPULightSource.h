#pragma once
#include "glm/vec3.hpp"

namespace GPU
{
#pragma pack(push, 1)
    struct GPULightSource {
        glm::vec3 position;      // 12 octets (0-11)
        float intensity;         // 4 octets  (12-15)
        glm::vec3 colorDiffuse;  // 12 octets (16-27)
        float halfWidth;         // 4 octets  (28-31)
        glm::vec3 colorSpecular; // 12 octets (32-43)
        float halfHeight;        // 4 octets  (44-47)
        glm::vec3 uDir;          // 12 octets (48-59)
        float _pad1;             // 4 octets  (60-63)
        glm::vec3 vDir;          // 12 octets (64-75)
        float _pad2;             // 4 octets  (76-79)
        // Total: 80 octets
    };
#pragma pack(pop)

    static_assert(sizeof(GPULightSource) == 80, "Taille doit être 80 octets");
    static_assert(offsetof(GPULightSource, position) == 0, "Position à offset 0");
    static_assert(offsetof(GPULightSource, intensity) == 12, "Intensity à offset 12");
    static_assert(offsetof(GPULightSource, colorDiffuse) == 16, "ColorDiffuse à offset 16");
}