uniform int numMaterials;

struct GPUMaterial {
    float reflectivity;    // 4 (0-3)
    float transparency;    // 4 (4-7)
    float shininess;       // 4 (8-11)
    float eta;             // 4 (12-15)

    float roughness;       // 4 (16-19)
    float metallic;        // 4 (20-23)
    float pad1;
    float pad2;
    vec3 f0;              // 12 (24-35)
    float pad3;
    vec3 k;
    float pas4;
};

layout(std430, binding = 3) buffer MaterialData {
    GPUMaterial materials[];
};