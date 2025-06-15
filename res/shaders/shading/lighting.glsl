#include "includes_structs.glsl"

vec3 computeDiffuse(vec3 color, vec3 normal, vec3 lightDir, GPULight light) {
    float NdotL = max(dot(normal, lightDir), 0.0);
    return color * light.colorDiffuse * NdotL;
}

vec3 computeSpecular(Ray ray, vec3 point, vec3 normal, GPULight light, int shapeIndex) {
    vec3 viewDir = normalize(ray.origin - point);
    vec3 lightDir = normalize(light.position - point);
    vec3 halfVec = normalize(viewDir + lightDir);

    float NdotH = max(dot(normal, halfVec), 0.0);
    GPUMaterial mat = materials[shapes[shapeIndex].materialIndex];

    return light.colorSpecular * light.intensity * pow(NdotH, mat.shininess);
}

// BRDF Cook-Torrance complet avec rugosité
vec3 computePBRSpecular(vec3 viewDir, vec3 lightDir, vec3 normal, GPUMaterial mat, GPULight light) {
    vec3 H = normalize(viewDir + lightDir);
    float NdotV = max(dot(normal, viewDir), 0.0);
    float NdotL = max(dot(normal, lightDir), 0.0);
    float HdotV = max(dot(H, viewDir), 0.0);

    // Distribution des micro-facettes
    float D = distributionGGX(normal, H, mat.roughness);

    // Fonction de géométrie
    float G = geometrySmith(normal, viewDir, lightDir, mat.roughness);

    // Fresnel
    vec3 F = fresnelSchlick(HdotV, mat.f0);

    vec3 numerator = D * G * F;
    float denominator = 4.0 * NdotV * NdotL + 0.0001; // +0.0001 pour éviter division par zéro

    return numerator / denominator;
}

vec3 computeSpecularPBR(Ray ray, vec3 point, vec3 normal, GPULight light, int shapeIndex) {
    vec3 viewDir = normalize(ray.origin - point);
    vec3 lightDir = normalize(light.position - point);

    GPUMaterial mat = materials[shapes[shapeIndex].materialIndex];

    // Utiliser le BRDF PBR complet
    vec3 specular = computePBRSpecular(viewDir, lightDir, normal, mat, light);

    float NdotL = max(dot(normal, lightDir), 0.0);
    return specular * light.colorSpecular * light.intensity * NdotL;
}


vec3 computeLighting(Ray ray, vec3 point, vec3 normal, vec3 color, int shapeIndex) {
    vec3 result = vec3(0.0);
    GPUMaterial mat = materials[shapes[shapeIndex].materialIndex];

    for (int i = 0; i < numLights; i++) {
        vec3 lightPos = lights[i].position;
        vec3 lightDir = normalize(lightPos - point);

        Ray shadowRay;
        shadowRay.origin = point + lightDir * EPSILON;
        vec3 toLight = lightPos - shadowRay.origin;
        float lightDist = length(toLight);
        shadowRay.direction = normalize(toLight);

        HitInfo shadowHit = findNearestIntersection(shadowRay);
        if (isInShadow(shadowHit, lightDist) && shadowsEnabled) {
            continue;
        }

        // Composante diffuse (Lambertian)
        vec3 diffuse = computeDiffuse(color, normal, shadowRay.direction, lights[i]);

        // Composante spéculaire PBR
        vec3 specular = vec3(0.0);
        if (specularEnabled) {
            if (roughnessEnabled) {
                specular = computeSpecularPBR(ray, point, normal, lights[i], shapeIndex);
            } else {
                specular = computeSpecular(ray, point, normal, lights[i], shapeIndex);
            }
        }

        vec3 finalColor;
        if (fresnelEnabled) {
            // Mélange diffus/spéculaire basé sur la métallicité et le Fresnel
            vec3 viewDir = normalize(ray.origin - point);
            float NdotV = max(dot(normal, viewDir), 0.0);
            vec3 F = fresnelSchlick(NdotV, mat.f0);

            vec3 kS = F;
            vec3 kD = vec3(1.0) - kS;
            kD *= 1.0 - mat.metallic; // Les métaux n'ont pas de composante diffuse

            finalColor = kD * diffuse + specular;
        } else {
            finalColor = diffuse + specular;
        }

        float attenuation = 1.0;
        if (attenuationEnabled) {
            attenuation = 1.0 / (1.0 + 0.1 * lightDist + 0.01 * lightDist * lightDist);
        }

        result += finalColor * attenuation;
    }
    return result;
}