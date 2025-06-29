uniform int numShapes;

struct GPUShape {
    vec3 color;         // 12 octets
    int type;           // 4 octets
    // Fin bloc 1: 16 octets

    int materialIndex;  // 4 octets
    int wireframeEnabled;
    float pad1_;       // 4 octets
    int textureIndex;        // 4 octets

    vec3 center;        // 12 octets
    float radius;       // 4 octets

    vec3 normal;        // 12 octets
    float dist;         // 4 octets

    vec3 A; //12 octets
    float pad2_;

    vec3 B;
    float pad3_;

    vec3 C;
    float pad4_;
};

layout(std430, binding = 1) buffer ShapeData {
    GPUShape shapes[];
};

#include "shapes/sphere.glsl"
#include "shapes/plane.glsl"
#include "shapes/triangle.glsl"

bool intersect(Ray ray, GPUShape shape, out float t) {
    if (shape.type == 0) { //Sphere
        return intersectSphere(ray, shape.center, shape.radius, t);
    } else if (shape.type == 1) { //Plane
        return intersectPlane(ray, shape.normal, shape.dist, t);
    } else if (shape.type == 2) { //Triangle
      return intersectTriangle(ray, shape.normal, shape.dist, shape.A, shape.B, shape.C, t);
    }
    return false;
};

vec3 getNormal(GPUShape shape, vec3 point) {
    if (shape.type == 0) { //Sphere
        return normalize(point - shape.center);
    } else if (shape.type == 1 || shape.type == 2) { //Plane or Triangle
        return normalize(shape.normal);
    }
    return vec3(0.0, 1.0, 0.0);
}

float getDistanceNearestEdge(Ray ray, vec3 point, int shapeIndex) {
    GPUShape shape = shapes[shapeIndex];
    if (shape.type == 0) {
        return getDistanceNearestEdgeSphere(ray, point, shape.center, shape.radius);
    } else if (shape.type == 1) {
        return wireframeThickness +1;
    } else if (shape.type == 2) {
        return getDistanceNearestEdgeTriangle(ray, point, shape.normal, shape.dist, shape.A, shape.B, shape.C);
    }
}

vec2 getTextureCoordinates(vec3 point, int shapeIndex) {
    GPUShape shape = shapes[shapeIndex];
    if (shape.type == 0) {
        return getTextureCoordinatesSphere(point, shape.center, shape.radius);
    } else if (shape.type == 1) {
        return getTextureCoordinatesPlane(point, shape.normal, shape.dist);
    } else if (shape.type == 2) {
        return getTextureCoordinatesTriangle(point, shape.normal, shape.dist, shape.A, shape.B, shape.C);
    }
}

