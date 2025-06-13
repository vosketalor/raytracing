uniform int numMaterials;

struct GPUMaterial {
    float reflectivity; // 4 octets
    float transparency; // 4 octets
    float shininess;    // 4 octets
    float eta;          // 4 octets
    float roughness;    // 4 octets
    float metallic;     // 4 octets
    vec3 f0;           // 12 octets
    float _pad;        // 4 octets
    // Total: 40 octets
};

layout(std430, binding = 3) buffer MaterialData {
    GPUMaterial materials[];
};