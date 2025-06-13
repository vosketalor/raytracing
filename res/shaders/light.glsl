uniform int numLights;

struct GPULight {
    vec3 position;
    vec3 colorDiffuse;
    vec3 colorSpecular;
    float intensity;

    vec3 uDir, vDir;
    float halfWidth, halfHeight;
};

layout(std430, binding = 2) buffer LightData {
    GPULight lights[];
};