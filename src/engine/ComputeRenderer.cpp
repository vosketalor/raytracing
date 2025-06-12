#include "ComputeRenderer.h"

#pragma once

#include <../../external/glew/auto/src/glew_head.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include "Camera.h"
#include "scenes/Scene.h"
#include "Vector.h"

class ComputeRenderer {
private:
    GLuint computeShader;
    GLuint shaderProgram;
    GLuint outputTexture;
    GLuint sceneDataSSBO;
    GLuint lightDataSSBO;
    GLuint materialDataSSBO;

    int width, height;
    Scene* scene;
    Camera camera_;

    struct GPUShape {
        float center[3];
        float radius;
        float color[3];
        int type;
        float material[4];
        float transform[16];
    };

    struct GPULight {
        float position[3];
        float intensity;
        float colorDiffuse[3];
        float pad1;
        float colorSpecular[3];
        float pad2;
        float area[2];
        float pad3[2];
    };

    struct GPUMaterial {
        float shininess;
        float eta;
        float f0[3];
        float pad;
    };

public:
    ComputeRenderer(Scene* scene, const Camera& camera, int width, int height);
    ~ComputeRenderer();

    bool initialize();
    void render(std::vector<Vector3>& frameBuffer);
    void setCamera(const Camera& camera);
    void cleanup();

private:
    bool loadComputeShader(const std::string& shaderSource);
    std::string loadShaderSource(const std::string& filename);
    void setupBuffers();
    void updateSceneData();
    void updateCameraUniforms();
};

ComputeRenderer::ComputeRenderer(Scene* scene, const Camera& camera, int width, int height)
    : scene(scene), camera_(camera), width(width), height(height),
      computeShader(0), shaderProgram(0), outputTexture(0),
      sceneDataSSBO(0), lightDataSSBO(0), materialDataSSBO(0) {
}

ComputeRenderer::~ComputeRenderer() {
    cleanup();
}

bool ComputeRenderer::initialize() {
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return false;
    }

    if (!GLEW_ARB_compute_shader) {
        std::cerr << "Compute shaders not supported" << std::endl;
        return false;
    }

    std::string shaderSource = R"(
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
)";

    return loadComputeShader(shaderSource);
}

bool ComputeRenderer::loadComputeShader(const std::string& shaderSource) {
    computeShader = glCreateShader(GL_COMPUTE_SHADER);
    const char* source = shaderSource.c_str();
    glShaderSource(computeShader, 1, &source, NULL);
    glCompileShader(computeShader);

    GLint success;
    glGetShaderiv(computeShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(computeShader, 512, NULL, infoLog);
        std::cerr << "Compute shader compilation failed:\n" << infoLog << std::endl;
        return false;
    }

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, computeShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "Shader program linking failed:\n" << infoLog << std::endl;
        return false;
    }

    setupBuffers();
    return true;
}

void ComputeRenderer::setupBuffers() {
    glGenTextures(1, &outputTexture);
    glBindTexture(GL_TEXTURE_2D, outputTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindImageTexture(0, outputTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    glGenBuffers(1, &sceneDataSSBO);
    glGenBuffers(1, &lightDataSSBO);
    glGenBuffers(1, &materialDataSSBO);

    updateSceneData();
}

void ComputeRenderer::updateSceneData() {
    std::vector<GPUShape> gpuShapes;
    for (const auto& shape : scene->getShapes()) {
        GPUShape gpuShape;

        gpuShape.center[0] = 0;
        gpuShape.center[1] = 0;
        gpuShape.center[2] = 0;
        gpuShape.radius = 1.0f;

        Vector3 color = shape->getColor();
        gpuShape.color[0] = color.x();
        gpuShape.color[1] = color.y();
        gpuShape.color[2] = color.z();

        gpuShape.type = 0;

        const Material& mat = shape->getMaterial();
        gpuShape.material[0] = mat.getRoughness();
        gpuShape.material[1] = mat.getMetallic();
        gpuShape.material[2] = mat.getTransparency();
        gpuShape.material[3] = mat.getReflectivity();

        gpuShapes.push_back(gpuShape);
    }

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, sceneDataSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, gpuShapes.size() * sizeof(GPUShape),
                 gpuShapes.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, sceneDataSSBO);

    std::vector<GPULight> gpuLights;
    for (const auto& light : scene->getLightSources()) {
        GPULight gpuLight;

        Vector3 pos = light->getPosition();
        gpuLight.position[0] = pos.x();
        gpuLight.position[1] = pos.y();
        gpuLight.position[2] = pos.z();
        gpuLight.intensity = light->getIntensity();

        Vector3 diffuse = light->getColorDiffuse();
        gpuLight.colorDiffuse[0] = diffuse.x();
        gpuLight.colorDiffuse[1] = diffuse.y();
        gpuLight.colorDiffuse[2] = diffuse.z();

        Vector3 specular = light->getColorSpecular();
        gpuLight.colorSpecular[0] = specular.x();
        gpuLight.colorSpecular[1] = specular.y();
        gpuLight.colorSpecular[2] = specular.z();

        gpuLights.push_back(gpuLight);
    }

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightDataSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, gpuLights.size() * sizeof(GPULight),
                 gpuLights.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, lightDataSSBO);
}

void ComputeRenderer::updateCameraUniforms() {
    glUseProgram(shaderProgram);

    Vector3 pos = camera_.getPosition();
    Vector3 dir = camera_.getDirection();
    Vector3 right = camera_.getRight();
    Vector3 up = camera_.getUp();

    glUniform3f(glGetUniformLocation(shaderProgram, "cameraPos"), pos.x(), pos.y(), pos.z());
    glUniform3f(glGetUniformLocation(shaderProgram, "cameraDir"), dir.x(), dir.y(), dir.z());
    glUniform3f(glGetUniformLocation(shaderProgram, "cameraRight"), right.x(), right.y(), right.z());
    glUniform3f(glGetUniformLocation(shaderProgram, "cameraUp"), up.x(), up.y(), up.z());
    glUniform1f(glGetUniformLocation(shaderProgram, "fov"), camera_.getFov());
    glUniform1f(glGetUniformLocation(shaderProgram, "aspectRatio"), (float)width / height);
    glUniform2f(glGetUniformLocation(shaderProgram, "resolution"), width, height);

    glUniform1i(glGetUniformLocation(shaderProgram, "numShapes"), scene->getShapes().size());
    glUniform1i(glGetUniformLocation(shaderProgram, "numLights"), scene->getLightSources().size());

    Vector3 skyColor = scene->getSkyColor();
    Vector3 ambient = scene->getAmbient();
    glUniform3f(glGetUniformLocation(shaderProgram, "skyColor"), skyColor.x(), skyColor.y(), skyColor.z());
    glUniform3f(glGetUniformLocation(shaderProgram, "ambientColor"), ambient.x(), ambient.y(), ambient.z());
}

void ComputeRenderer::render(std::vector<Vector3>& frameBuffer) {
    updateCameraUniforms();

    glUseProgram(shaderProgram);

    int groupsX = (width + 15) / 16;
    int groupsY = (height + 15) / 16;
    glDispatchCompute(groupsX, groupsY, 1);

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    glBindTexture(GL_TEXTURE_2D, outputTexture);
    std::vector<float> pixels(width * height * 4);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, pixels.data());

    frameBuffer.resize(width * height);
    for (int i = 0; i < width * height; i++) {
        frameBuffer[i] = Vector3(pixels[i*4], pixels[i*4+1], pixels[i*4+2]);
    }
}

void ComputeRenderer::setCamera(const Camera& camera) {
    camera_ = camera;
}

void ComputeRenderer::cleanup() {
    if (outputTexture) glDeleteTextures(1, &outputTexture);
    if (sceneDataSSBO) glDeleteBuffers(1, &sceneDataSSBO);
    if (lightDataSSBO) glDeleteBuffers(1, &lightDataSSBO);
    if (materialDataSSBO) glDeleteBuffers(1, &materialDataSSBO);
    if (shaderProgram) glDeleteProgram(shaderProgram);
    if (computeShader) glDeleteShader(computeShader);
}