#pragma once
#include "glm/vec3.hpp"

namespace GPU
{
#pragma pack(push, 1)
    struct GPUMaterial {
        float reflectivity;  // 4 (0-3)
        float transparency;  // 4 (4-7)
        float shininess;     // 4 (8-11)
        float eta;           // 4 (12-15)

        float roughness;     // 4 (16-19)
        float metallic;      // 4 (20-23)
        float pad1_=0;
        float pad2_=0;
        glm::vec3 f0;        // 12 (24-35)
        float pad3_=0;
        // Total: 36 octets
    };
#pragma pack(pop)
    static_assert(sizeof(GPUMaterial) % 16 == 0, "Doit être aligné sur 16 octets");
    static_assert(offsetof(GPUMaterial, f0) % 16 == 0, "Vec3 doit être aligné sur 16 octets");
}
