#include "includes_structs.glsl"

Ray spawnBounceRay(vec3 origin, vec3 direction) {
    Ray bounce;
    bounce.origin = origin + direction * EPSILON;
    bounce.direction = direction;
    return bounce;
}

vec3 traceRay(Ray ray) {
    vec3 finalColor = vec3(0.0);
    vec3 throughput = vec3(1.0);

    for (int depth = 0; depth < MAX_BOUNCES; ++depth) {
        HitInfo hit;
        if (useBVH) {
            hit = findNearestIntersectionBVH(ray);
        } else {
            hit = findNearestIntersection(ray);
        }

        if (!hit.hit) {
            finalColor += skyColor * throughput;
            break;
        }

        GPUMaterial mat = materials[shapes[hit.shapeIndex].materialIndex];

        // Couleur locale
        vec3 localColor = hit.color * ambientColor;
        localColor += computeLighting(ray, hit.point, hit.normal, hit.color, hit.shapeIndex);

        // Calculer Fresnel (retourne un vec3)
        vec3 fresnelRGB = computeFresnel(ray.direction, hit.normal, 1.0, mat.eta, mat.f0, mat.metallic);

        bool hasTransparency = (mat.transparency > 0.0 ? true : false) && refractionsEnabled;
        bool hasReflectivity = (mat.reflectivity > 0.0 ? true : false) && reflectionsEnabled;

        if (hasTransparency && hasReflectivity && fresnelEnabled) {
            // Mélange réflexion/transmission avec couleur Fresnel
            vec3 reflectionAmount = fresnelRGB * mat.reflectivity;
            float transmissionAmount = (1.0 - dot(fresnelRGB, vec3(0.333))) * mat.transparency;
            vec3 localAmount = vec3(1.0) - reflectionAmount - vec3(transmissionAmount);

            // Contribution locale
            finalColor += localColor * throughput * localAmount;

            // Décision basée sur l'intensité moyenne du Fresnel
            float avgFresnel = dot(fresnelRGB, vec3(0.333));
            if (avgFresnel * mat.reflectivity > transmissionAmount) {
                // Réflexion avec couleur Fresnel
                vec3 reflectDir;
                if (roughnessEnabled) {
                    vec2 seed = vec2(float(gl_GlobalInvocationID.x), float(gl_GlobalInvocationID.y)) * EPSILON;
                    reflectDir = computeReflectionDirectionRough(ray.direction, hit.normal, mat.roughness, seed);
                } else {
                    reflectDir = computeReflection(ray, hit.point, hit.normal, hit.shapeIndex, depth);
                }

                ray = spawnBounceRay(hit.point, reflectDir);
                throughput *= reflectionAmount;
            } else {
                // Transmission
                vec3 refractDir = computeRefractionDirection(ray.direction, hit.normal, 1.0, mat.eta);
                ray = spawnBounceRay(hit.point, refractDir);
                throughput *= transmissionAmount;
            }
        }
        else if (hasReflectivity) {
            vec3 reflectionAmount = fresnelRGB * mat.reflectivity;
            finalColor += localColor * throughput * (vec3(1.0) - reflectionAmount);

            vec3 reflectDir;
            if (roughnessEnabled) {
                vec2 seed = vec2(float(gl_GlobalInvocationID.x), float(gl_GlobalInvocationID.y)) * EPSILON;
                reflectDir = computeReflectionDirectionRough(ray.direction, hit.normal, mat.roughness, seed);
            } else {
                reflectDir = computeReflection(ray, hit.point, hit.normal, hit.shapeIndex, depth);
            }

            ray = spawnBounceRay(hit.point, reflectDir);
            throughput *= reflectionAmount;
        }
        else if (hasTransparency) {
            float avgFresnel = dot(fresnelRGB, vec3(0.333));
            float transmissionAmount = (1.0 - avgFresnel) * mat.transparency;
            finalColor += localColor * throughput * (1.0 - transmissionAmount);

            vec3 refractDir = computeRefractionDirection(ray.direction, hit.normal, 1.0, mat.eta);
            ray = spawnBounceRay(hit.point, refractDir);
            throughput *= transmissionAmount;
        }
        else {
            finalColor += localColor * throughput;
            break;
        }

        // Éviter les bounces infinitésimaux
        if (dot(throughput, throughput) < EPSILON) break;
    }

    return finalColor;
}