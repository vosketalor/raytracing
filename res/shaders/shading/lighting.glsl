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

vec3 computeLighting(Ray ray, vec3 point, vec3 normal, vec3 color, int shapeIndex) {
    vec3 result = vec3(0.0);

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

        vec3 diffuse = computeDiffuse(color, normal, shadowRay.direction, lights[i]);

        vec3 specular = vec3(0.0);
        if (specularEnabled) {
            specular = computeSpecular(ray, point, normal, lights[i], shapeIndex);
        }

        float attenuation = 1.0;
        if (attenuationEnabled) {
            attenuation = 1.0 / (1.0 + 0.1 * lightDist + 0.01 * lightDist * lightDist);
        }

        result += (diffuse + specular) * attenuation;
    }

    return result;
}
