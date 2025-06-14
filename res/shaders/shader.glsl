#version 430

layout(local_size_x = 16, local_size_y = 16) in;
layout(rgba32f, binding = 0) uniform image2D outputImage;

uniform vec3 cameraPos;
uniform vec3 cameraDir;
uniform vec3 cameraRight;
uniform vec3 cameraUp;
uniform float fov;
uniform float aspectRatio;
uniform ivec2 resolution;

uniform vec3 skyColor;
uniform vec3 ambientColor;

const float EPSILON = 1e-4f;
const float MAX_DIST = 1e6f;
const int MAX_BOUNCES = 10;

#include "hitinfo.glsl"
#include "ray.glsl"
#include "shapes/shapes.glsl"
#include "material.glsl"
#include "light.glsl"

HitInfo findNearestIntersection(Ray ray) {
    HitInfo hit;
    hit.hit = false;
    hit.t = MAX_DIST;

    for (int i = 0; i < numShapes; i++) {
        float t;
        GPUShape shape = shapes[i];
        bool intersected = intersect(ray, shape, t);

        if (intersected && t > EPSILON && t < hit.t) {
            hit.hit = true;
            hit.t = t;
            hit.point = ray.origin + ray.direction * t;
            hit.normal = getNormal(shape, hit.point);
            hit.color = vec3(shape.color[0], shape.color[1], shape.color[2]);
            hit.shapeIndex = i;
        }
    }

    return hit;
}

vec3 computeLighting(vec3 point, vec3 normal, vec3 viewDir, vec3 color, int shapeIndex) {
    vec3 result = vec3(0.0);

    for (int i = 0; i < numLights; i++) {
        vec3 lightPos = lights[i].position;
        vec3 lightDir = normalize(lightPos - point);
        float lightDist = length(lightPos - point);

        // Test d'ombre avec debug
        Ray shadowRay;
        shadowRay.origin = point + normal * EPSILON;
        shadowRay.direction = lightDir;

        HitInfo shadowHit = findNearestIntersection(shadowRay);

        bool inShadow = false;
        if (shadowHit.hit) {
            // Distance à l'obstacle
            float obstacleDistance = shadowHit.t;

            // Vérifier si l'obstacle est entre le point et la lumière
            if (obstacleDistance < lightDist - EPSILON) {
                // Vérifier que ce n'est pas le même objet (auto-ombrage)
                if (shadowHit.shapeIndex != shapeIndex) {
                    inShadow = true;
                }
            }
        }

        if (inShadow) {
            continue;  // Dans l'ombre, pas de contribution de cette lumière
        }

        float NdotL = max(dot(normal, lightDir), 0.0);
        if (NdotL <= 0.0) continue;

        // Composante diffuse
        vec3 diffuse = color * lights[i].colorDiffuse * NdotL;

        // Composante spéculaire (Blinn-Phong)
        vec3 specular = vec3(0.0);
        vec3 halfVec = normalize(viewDir + lightDir);
        float NdotH = max(dot(normal, halfVec), 0.0);
        if (NdotH > 0.0) {
            GPUMaterial mat = materials[shapes[shapeIndex].materialIndex];
            specular = lights[i].colorSpecular * pow(NdotH, mat.shininess);
        }

        // Atténuation de la distance
        float attenuation = 1.0 / (1.0 + 0.1 * lightDist + 0.01 * lightDist * lightDist);

        result += (diffuse + specular) * attenuation;
    }

    return result;
}

vec3 computeReflectionDirection(vec3 dir, vec3 normal) {
    return reflect(dir, normal);
}

vec3 computeRefractionDirection(vec3 dir, vec3 normal, float etaI, float etaT, out bool tir) {
    float cosi = clamp(dot(dir, normal), -1.0, 1.0);
    float eta = etaI / etaT;
    vec3 n = normal;
    if (cosi < 0.0) {
        cosi = -cosi;
    } else {
        n = -normal;
        eta = etaT / etaI;
    }

    float k = 1.0 - eta * eta * (1.0 - cosi * cosi);
    if (k < 0.0) {
        tir = true;
        return reflect(dir, normal); // fallback to reflection
    } else {
        tir = false;
        return normalize(eta * dir + (eta * cosi - sqrt(k)) * n);
    }
}

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
            finalColor += attenuation * skyColor;
            break;
        }

        GPUMaterial mat = materials[shapes[hit.shapeIndex].materialIndex];
        vec3 viewDir = -ray.direction;

        // Éclairage direct
        vec3 localColor = hit.color * ambientColor;
        localColor += computeLighting(hit.point, hit.normal, viewDir, hit.color, hit.shapeIndex);
        finalColor += attenuation * localColor;

        // Réflexion ou réfraction
        bool reflected = (mat.reflectivity > 0.0) ? true : false;
        bool refracted = (mat.transparency > 0.0) ? true : false;

        if (!reflected && !refracted) break;

        vec3 nextDir;
        bool tir = false;

        if (refracted) {
            nextDir = computeRefractionDirection(ray.direction, hit.normal, 1.0, mat.eta, tir);
            if (!tir) {
                ray = spawnBounceRay(hit.point, nextDir);
                attenuation *= mat.transparency;
                continue;
            }
        }

        if (reflected) {
            nextDir = computeReflectionDirection(ray.direction, hit.normal);
            ray = spawnBounceRay(hit.point, nextDir);
            attenuation *= mat.reflectivity;
            continue;
        }

        break;
    }

    return finalColor;
}


void main() {
    ivec2 pixelCoord = ivec2(gl_GlobalInvocationID.xy);
    if (pixelCoord.x >= resolution.x || pixelCoord.y >= resolution.y) {
        return;
    }

    vec2 uv = (vec2(pixelCoord) + 0.5) / vec2(resolution);
    vec2 ndc = uv * 2.0 - 1.0;

    float fovRad = fov * 3.14159265359 / 180.0;
    float screenHeight = 2.0 * tan(fovRad / 2.0);
    float screenWidth = screenHeight * aspectRatio;

    vec3 rayDir = normalize(cameraDir +
                           ndc.x * screenWidth * 0.5 * cameraRight +
                           ndc.y * screenHeight * 0.5 * cameraUp);

    Ray ray;
    ray.origin = cameraPos;
    ray.direction = rayDir;

    vec3 color = traceRay(ray);

    imageStore(outputImage, pixelCoord, vec4(color, 1.0));
}