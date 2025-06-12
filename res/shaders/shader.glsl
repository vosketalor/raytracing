#version 430

layout(local_size_x = 16, local_size_y = 16) in;
layout(rgba32f, binding = 0) uniform image2D outputImage;

uniform vec3 cameraPos;
uniform vec3 cameraDir;
uniform vec3 cameraRight;
uniform vec3 cameraUp;
uniform float fov;
uniform float aspectRatio;
uniform vec2 resolution;

layout(std430, binding = 1) buffer SceneData {
    GPUShape shapes[];
};

layout(std430, binding = 2) buffer LightData {
    GPULight lights[];
};

layout(std430, binding = 3) buffer MaterialData {
    GPUMaterial materials[];
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

bool intersectSphere(Ray ray, vec3 center, float radius, out float t) {
    vec3 oc = ray.origin - center;
    float a = dot(ray.direction, ray.direction);
    float b = 2.0 * dot(oc, ray.direction);
    float c = dot(oc, oc) - radius * radius;

    float discriminant = b * b - 4.0 * a * c;
    if (discriminant < 0.0) return false;

    float sqrt_disc = sqrt(discriminant);
    float t1 = (-b - sqrt_disc) / (2.0 * a);
    float t2 = (-b + sqrt_disc) / (2.0 * a);

    t = (t1 > EPSILON) ? t1 : t2;
    return t > EPSILON;
}

bool intersectPlane(Ray ray, vec3 normal, vec3 point, out float t) {
    float denom = dot(normal, ray.direction);
    if (abs(denom) < EPSILON) return false;

    t = dot(point - ray.origin, normal) / denom;
    return t > EPSILON;
}

HitInfo findNearestIntersection(Ray ray) {
    HitInfo hit;
    hit.hit = false;
    hit.t = MAX_DIST;

    for (int i = 0; i < numShapes; i++) {
        float t;
        bool intersected = false;

        if (shapes[i].type == 0) {
            vec3 center = vec3(shapes[i].center[0], shapes[i].center[1], shapes[i].center[2]);
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

vec3 traceRay(Ray ray, int depth) {
    if (depth >= MAX_BOUNCES) return skyColor;

    HitInfo hit = findNearestIntersection(ray);
    if (!hit.hit) return skyColor;

    vec3 viewDir = -ray.direction;
    vec3 color = computeLighting(hit.point, hit.normal, viewDir, hit.color, hit.shapeIndex);

    float reflectivity = shapes[hit.shapeIndex].material[3];
    if (reflectivity > 0.0 && depth < MAX_BOUNCES - 1) {
        vec3 reflectDir = reflect(ray.direction, hit.normal);
        Ray reflectRay;
        reflectRay.origin = hit.point + hit.normal * EPSILON;
        reflectRay.direction = reflectDir;

        vec3 reflectColor = traceRay(reflectRay, depth + 1);
        color = mix(color, reflectColor, reflectivity);
    }

    return color;
}

void main() {
    ivec2 pixelCoord = ivec2(gl_GlobalInvocationID.xy);
    if (pixelCoord.x >= int(resolution.x) || pixelCoord.y >= int(resolution.y)) {
        return;
    }

    vec2 uv = (vec2(pixelCoord) + 0.5) / resolution;
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

    vec3 color = traceRay(ray, 0);

    imageStore(outputImage, pixelCoord, vec4(color, 1.0));
}