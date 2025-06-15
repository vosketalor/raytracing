uniform int numShapes;

struct GPUShape {
    vec3 color;         // 12 octets
    int type;           // 4 octets
    // Fin bloc 1: 16 octets

    int materialIndex;  // 4 octets
    float _pad1;        // 4 octets
    float _pad2;        // 4 octets
    float _pad3;        // 4 octets
    // Fin bloc 2: 16 octets

    vec3 center;        // 12 octets
    float radius;       // 4 octets
    // Fin bloc 3: 16 octets

    vec3 normal;        // 12 octets
    float dist;         // 4 octets
    // Fin bloc 4: 16 octets
    // Total: 64 octets
};

layout(std430, binding = 1) buffer ShapeData {
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
    if (shape.type == 0) { //Sphere
        return normalize(point - shape.center);
    } else if (shape.type == 1) { //Plane
        return normalize(shape.normal);
    }
    return vec3(0.0, 1.0, 0.0);
}

