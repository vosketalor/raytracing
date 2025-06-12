uniform int numLights;

struct GPULight {
    vec3 position;
    float intensity;
    vec3 colorDiffuse;
    float pad1;       // padding pour alignement
    vec3 colorSpecular;
    float pad2;       // padding
    vec2 area;
    vec2 pad3;        // padding
};

layout(std430, binding = 2) buffer LightData {
    GPULight lights[];
};