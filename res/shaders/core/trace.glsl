#include "includes_structs.glsl"

Ray spawnBounceRay(vec3 origin, vec3 direction) {
    Ray bounce;
    bounce.origin = origin + direction * EPSILON;
    bounce.direction = direction;
    return bounce;
}

vec3 traceRay(Ray ray) {
    vec3 finalColor = vec3(0.0);
    vec3 attenuation = vec3(1.0);

    for (int depth = 0; depth < MAX_BOUNCES; ++depth) {

        HitInfo hit = findNearestIntersection(ray);
        if (!hit.hit) {
            finalColor = skyColor * attenuation;
            break;
        }

        GPUMaterial mat = materials[shapes[hit.shapeIndex].materialIndex];

        vec3 localColor = hit.color * ambientColor;
        localColor += computeLighting(ray, hit.point, hit.normal, hit.color, hit.shapeIndex);
        finalColor += localColor * attenuation;

        bool hasReflection = (mat.reflectivity > 0.0) ? true : false;
        bool hasRefraction = (mat.transparency > 0.0) ? true : false;

        if (!hasReflection && !hasRefraction) break;

        if (hasRefraction && refractionsEnabled) {
            vec3 refractDir = computeRefraction(ray, hit.point, hit.normal, hit.shapeIndex, depth);

            ray = spawnBounceRay(hit.point, refractDir);
            attenuation *= mat.transparency;
            continue;
        }

        if (hasReflection && reflectionsEnabled) {
            vec3 reflectDir = computeReflection(ray, hit.point, hit.normal, hit.shapeIndex, depth);

            ray = spawnBounceRay(hit.point, reflectDir);
            attenuation *= mat.reflectivity;
            continue;
        }

        break;
    }

    return finalColor;
}