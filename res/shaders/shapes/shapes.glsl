uniform int numShapes;

struct GPUShape {
    vec3 color;
    int type;

    //Sphere
    vec3 center;
    float radius;

    //Plane
    vec3 normal;
    float dist;

};

layout(std430, binding = 1) buffer SceneData {
    GPUShape shapes[];
};

#include "shapes/sphere.glsl"
#include "shapes/plane.glsl"

bool intersect(Ray ray, GPUShape shape, out float t) {
    if (shape.type == 0) { //Sphere
        return intersectSphere(ray, shape.center, shape.radius, t);
    } else if (shape.type == 1) { //Plane
        return intersectPlane(ray, shape.normal, shape.dist, t);
    }
    return false;
};

vec3 getNormal(GPUShape shape, vec3 point) {
    if (shape.type == 0) {
        return normalize(point - shape.center);
    } else if (shape.type == 1) {
        return normalize(shape.normal);
    }
}

