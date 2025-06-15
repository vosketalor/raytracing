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

vec3 computeReflectionDirectionRough(vec3 dir, vec3 normal, float roughness, vec2 randomSeed) {
    if (roughness < 0.001) {
        // Réflection parfaite pour les surfaces très lisses
        return reflect(dir, normal);
    }

    // Échantillonnage importance avec GGX
    vec2 xi = vec2(random(randomSeed), random(randomSeed + vec2(1.0, 0.0)));
    vec3 H = sampleGGX(normal, roughness, xi);

    // Direction de réflection basée sur la micro-facette échantillonnée
    return reflect(dir, H);
}