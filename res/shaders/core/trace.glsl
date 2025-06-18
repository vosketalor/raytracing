#include "includes_structs.glsl"

Ray spawnBounceRay(vec3 origin, vec3 direction) {
    Ray bounce;
    bounce.origin = origin + normalize(direction) * EPSILON;
    bounce.direction = normalize(direction);
    return bounce;
}

vec3 traceRay(Ray ray) {
    vec3 finalColor = vec3(0.0);
    vec3 throughput = vec3(1.0);

    for (int depth = 0; depth < MAX_BOUNCES; ++depth) {
        HitInfo hit = useBVH ? findNearestIntersectionBVH(ray) : findNearestIntersection(ray);

        if (!hit.hit) {
            finalColor += throughput * skyColor;
            break;
        }

        GPUMaterial mat = materials[shapes[hit.shapeIndex].materialIndex];

        vec3 localColor = hit.color * ambientColor;
        localColor += computeLighting(ray, hit.point, hit.normal, hit.color, hit.shapeIndex);

        bool isReflective = reflectionsEnabled && (mat.reflectivity > 0.0);
        bool isTransparent = refractionsEnabled && (mat.transparency > 0.0);

        if (!isReflective && !isTransparent) {
            finalColor += throughput * localColor;
            break;
        }

        if (isReflective && !isTransparent) {
            finalColor += throughput * localColor * (1.0 - mat.reflectivity);

            vec3 reflectDir = computeReflection(ray, hit.point, hit.normal, hit.shapeIndex, depth);
            throughput *= mat.reflectivity;
            ray = spawnBounceRay(hit.point, reflectDir);
            continue;
        }

        if (!isReflective && isTransparent) {
            finalColor += throughput * localColor * (1.0 - mat.transparency);

            vec3 refractDir = computeRefraction(ray, hit.point, hit.normal, hit.shapeIndex);
            throughput *= mat.transparency;
            ray = spawnBounceRay(hit.point, refractDir);
            continue;
        }

        if (dot(throughput, throughput) < EPSILON && depth > 2) break;
    }

    return finalColor;
}