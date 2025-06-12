uniform int numShapes;

struct GPUShape {
    vec3 center;      // float[3]
    float radius;
    vec3 color;
    int type;         // int (32 bits)
    vec4 material;    // float[4]
    mat4 transform;   // float[16]
};

layout(std430, binding = 1) buffer SceneData {
    GPUShape shapes[];
};

#include "shapes/sphere.glsl"
#include "shapes/plane.glsl"

