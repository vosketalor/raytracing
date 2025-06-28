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

        //Wireframe
        if (shapes[hit.shapeIndex].wireframeEnabled != 0 && depth == 0) {
            vec3 color;
            if (handleWireframe(ray, hit.point, hit.shapeIndex, color)) {
                finalColor += throughput * color;
                break;
            }
        }

        //Texture
        vec3 color;
        if (shapes[hit.shapeIndex].hasTexture != 0) {
            vec2 uv = getTextureCoordinates(hit.point, hit.shapeIndex);
            color = getColorTexture(uv, shapes[hit.shapeIndex].textureIndex);
        } else {
            color = hit.color;
        }
//        vec2 uv = getTextureCoordinates(hit.point, hit.shapeIndex);
//        color = getColorTexture(uv);

        GPUMaterial mat = materials[shapes[hit.shapeIndex].materialIndex];

        vec3 localColor = color * ambientColor;
        localColor += computeLighting(ray, hit.point, hit.normal, color, hit.shapeIndex);

        bool isReflective = reflectionsEnabled && (mat.reflectivity > 0.0);
        bool isTransparent = refractionsEnabled && (mat.transparency > 0.0);

        if (!isReflective && !isTransparent) {
            finalColor += throughput * localColor;
            break;
        }

        if (isReflective && isTransparent && fresnelEnabled) {
            vec3 fresnelCoeff = computeFresnel(
                ray.direction,
                hit.normal,
                1.0,
                vec3(mat.eta),
                mat.k,
                mat.metallic
            );

            // Contribution locale
            float localWeight = 1.0 - max(mat.reflectivity, mat.transparency);
            finalColor += throughput * localColor * localWeight;

            // Choix déterministe basé sur Fresnel
            float avgFresnel = dot(fresnelCoeff, vec3(0.3333));

            if (0.5 < avgFresnel) {
                // Réflexion
                vec3 reflectDir = computeReflection(ray, hit.point, hit.normal, hit.shapeIndex, depth);
                throughput *= fresnelCoeff * mat.reflectivity * 2.0;
                ray = spawnBounceRay(hit.point, reflectDir);
            } else {
                // Transmission
                vec3 refractDir = computeRefractionDirection(ray.direction, hit.normal, 1.0, mat.eta);
                vec3 transmissionCoeff = vec3(1.0) - fresnelCoeff;
                throughput *= transmissionCoeff * mat.transparency * 2.0;
                ray = spawnBounceRay(hit.point, refractDir);
            }
            continue;
        } else if (isReflective) {
            finalColor += throughput * localColor * (1.0 - mat.reflectivity);

            vec3 reflectDir = computeReflection(ray, hit.point, hit.normal, hit.shapeIndex, depth);
            throughput *= mat.reflectivity;
            ray = spawnBounceRay(hit.point, reflectDir);
            continue;
        } else if (isTransparent) {
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