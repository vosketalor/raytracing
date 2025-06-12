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

struct GPULight {
    vec3 position;
    float intensity;
    vec3 colorDiffuse;
    float pad1;       // padding pour alignement
    vec3 colorSpecular;
    float pad2;       // padding
    vec2 area;
    vec2 pad3;        // padding
};

struct GPUMaterial {
    float shininess;
    float eta;
    vec3 f0;
    float pad;        // padding
};

uniform int numShapes;
uniform int numLights;
uniform vec3 skyColor;
uniform vec3 ambientColor;

struct Ray {
    vec3 origin;
    vec3 direction;
};

struct HitInfo {
    bool hit;
    float t;
    vec3 point;
    vec3 normal;
    vec3 color;
    int shapeIndex;
};

const float EPSILON = 1e-6;
const float MAX_DIST = 1e6;
const int MAX_BOUNCES = 10;

#include "shapes/shapes.glsl"

layout(std430, binding = 1) buffer SceneData {
    GPUShape shapes[];
};

layout(std430, binding = 2) buffer LightData {
    GPULight lights[];
};

layout(std430, binding = 3) buffer MaterialData {
    GPUMaterial materials[];
};

HitInfo findNearestIntersection(Ray ray) {
    HitInfo hit;
    hit.hit = false;
    hit.t = MAX_DIST;

    for (int i = 0; i < numShapes; i++) {
        float t;
        bool intersected = false;

        if (shapes[i].type == 0) {
//            vec3 center = vec3(shapes[i].center[0], shapes[i].center[1], shapes[i].center[2]);
            vec3 center = shapes[i].center;

            intersected = intersectSphere(ray, center, shapes[i].radius, t);

            if (intersected && t < hit.t) {
                hit.hit = true;
                hit.t = t;
                hit.point = ray.origin + ray.direction * t;
                hit.normal = normalize(hit.point - center);
                hit.color = vec3(shapes[i].color[0], shapes[i].color[1], shapes[i].color[2]);
                hit.shapeIndex = i;
            }
        }
    }

    return hit;
}

vec3 computeLighting(vec3 point, vec3 normal, vec3 viewDir, vec3 color, int shapeIndex) {
    vec3 result = ambientColor * color;

    for (int i = 0; i < numLights; i++) {
        vec3 lightPos = vec3(lights[i].position[0], lights[i].position[1], lights[i].position[2]);
        vec3 lightDir = normalize(lightPos - point);
        float lightDist = length(lightPos - point);

        Ray shadowRay;
        shadowRay.origin = point + normal * EPSILON;
        shadowRay.direction = lightDir;

        HitInfo shadowHit = findNearestIntersection(shadowRay);
        if (shadowHit.hit && shadowHit.t < lightDist) {
            continue;
        }

        float NdotL = max(dot(normal, lightDir), 0.0);
        vec3 diffuse = color * vec3(lights[i].colorDiffuse[0], lights[i].colorDiffuse[1], lights[i].colorDiffuse[2]) * NdotL;

        vec3 halfVec = normalize(viewDir + lightDir);
        float NdotH = max(dot(normal, halfVec), 0.0);
        vec3 specular = vec3(lights[i].colorSpecular[0], lights[i].colorSpecular[1], lights[i].colorSpecular[2]) *
                       pow(NdotH, 32.0) * lights[i].intensity;

        float attenuation = 1.0 / (1.0 + 0.1 * lightDist + 0.01 * lightDist * lightDist);

        result += (diffuse + specular) * attenuation;
    }

    return result;
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

        vec3 viewDir = -ray.direction;
        vec3 localColor = computeLighting(hit.point, hit.normal, viewDir, hit.color, hit.shapeIndex);
        finalColor += attenuation * localColor;

        float reflectivity = shapes[hit.shapeIndex].material.w;
        if (reflectivity <= 0.0) {
            break;
        }

        vec3 reflectDir = reflect(ray.direction, hit.normal);
        ray.origin = hit.point + hit.normal * EPSILON;
        ray.direction = reflectDir;
        attenuation *= reflectivity;
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
    ndc.x *= aspectRatio;

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