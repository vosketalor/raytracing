uniform int numLights;

struct GPULight {
    vec3 position;      // 12 octets
    float intensity;    // 4 octets
    vec3 colorDiffuse;  // 12 octets
    float halfWidth;    // 4 octets
    vec3 colorSpecular; // 12 octets
    float halfHeight;   // 4 octets
    vec3 uDir;          // 12 octets
    float _pad1;        // 4 octets
    vec3 vDir;          // 12 octets
    float _pad2;        // 4 octets
    // Total: 80 octets
};

layout(std430, binding = 2) buffer LightData {
    GPULight lights[];
};