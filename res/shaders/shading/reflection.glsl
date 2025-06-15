#include "includes_structs.glsl"

vec3 computeReflectionDirection(vec3 dir, vec3 normal) {
    return reflect(dir, normal);
}

vec3 computeReflection(Ray ray, vec3 point, vec3 normal, int shapeIndex, int order) {
    if (order <= 0) return vec3(0.0);

    GPUMaterial mat = materials[shapes[shapeIndex].materialIndex];
    if (mat.reflectivity <= 0.0) return vec3(0.0);

    return computeReflectionDirection(ray.direction, normal);
}