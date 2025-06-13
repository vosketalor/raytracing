#pragma once
#include "glm/vec3.hpp"

namespace GPU
{
#pragma pack(push, 1)
    struct GPUMaterial {
        float reflectivity=0;     // 4 octets (0-3)
        float transparency=0;     // 4 octets (4-7)
        float shininess=0;        // 4 octets (8-11)
        float eta=0;              // 4 octets (12-15)
        float roughness=0;        // 4 octets (16-19)
        float metallic=0;         // 4 octets (20-23)
        glm::vec3 f0 = glm::vec3(0,0,0);          // 12 octets (24-35)
        float _pad = 0;            // 4 octets (36-39) - padding pour arrondir à 40
        // Total: 40 octets
    };
#pragma pack(pop)

    static_assert(sizeof(GPUMaterial) == 40, "Taille doit être 40 octets");
    static_assert(offsetof(GPUMaterial, f0) == 24, "f0 à offset 24");
}
