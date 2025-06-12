uniform int numShapes;

struct GPUShape {
    vec3 color;
    int type;

    //Sphere
    vec3 center;
    float radius;
};

layout(std430, binding = 1) buffer SceneData {
    GPUShape shapes[];
};

#include "shapes/sphere.glsl"
#include "shapes/plane.glsl"

