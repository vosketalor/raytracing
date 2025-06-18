#include "includes_structs.glsl"

vec3 computeReflectionDirection(vec3 dir, vec3 normal) {
    return reflect(dir, normal);
}

vec3 computeReflectionDirectionRough(vec3 incident, vec3 normal, float roughness, vec2 randomSeed) {
    if (roughness < 0.001) {
        return reflect(incident, normal);
    }

    // Génération de nombres aléatoires
    vec2 xi = vec2(
    random(randomSeed),
    random(randomSeed + vec2(1.0, 0.0))
    );

    // Échantillonnage GGX
    vec3 H = sampleGGX(normal, roughness, xi);

    // Direction de réflexion par rapport à la micro-facette
    return reflect(incident, H);
}

vec3 computeReflection(Ray ray, vec3 point, vec3 normal, int shapeIndex, int order) {
    // Supprimez toutes les conditions de garde - elles sont vérifiées dans traceRay()
    GPUMaterial mat = materials[shapes[shapeIndex].materialIndex];

    vec3 reflectDir;
    if (roughnessEnabled) {
        vec2 randomSeed = vec2(float(gl_GlobalInvocationID.x + shapeIndex * 17),
        float(gl_GlobalInvocationID.y + order * 73));
        reflectDir = computeReflectionDirectionRough(ray.direction, normal, mat.roughness, randomSeed);
    } else {
        reflectDir = computeReflectionDirection(ray.direction, normal);
    }

    return reflectDir;
}
