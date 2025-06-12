struct GPUMaterial {
    float shininess;
    float eta;
    vec3 f0;
    float pad;        // padding
};

layout(std430, binding = 3) buffer MaterialData {
    GPUMaterial materials[];
};