#include "includes_structs.glsl"

vec3 computeRefractionDirection(vec3 dir, vec3 normal, float etaI, float etaT) {
    vec3 i = normalize(dir);
    float c1 = dot(normal, i);
    bool exiting = (c1 >= 0.0) ? true : false;

    float eta;
    vec3 n = normal;

    if (exiting) {
        // Sortie du matériau
        n = -normal;
        eta = etaT / etaI;
    } else {
        // Entrée dans le matériau
        c1 = -c1;
        eta = etaI / etaT;
    }

    float sin2ThetaI = 1.0 - c1 * c1;
    float sin2ThetaT = eta * eta * sin2ThetaI;
    float k = 1.0 - sin2ThetaT;

    if (k >= 0.0) {
        float c2 = sqrt(k);
        return normalize(i * eta + n * (eta * c1 - c2));
    } else {
        return computeReflectionDirection(i, normal);
    }
}

vec3 computeRefraction(Ray ray, vec3 point, vec3 normal, int shapeIndex) {
    GPUMaterial mat = materials[shapes[shapeIndex].materialIndex];

    return computeRefractionDirection(ray.direction, normal, 1.0, mat.eta);
}