#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>


#define IMGUI_IMPL_OPENGL_LOADER_GLEW
#include "imgui_impl_opengl3.h"   // uniquement ce fichier ImGui

#include <vector>
#include <string>

#include <unordered_set>

#include "Camera.h"
#include "scenes/Scene.h"
#include "Vector.h"
#include "../preferences/PreferenceManager.h"
#include "acceleration/BVHNode.h"

class ComputeRenderer {
private:
    GLuint computeShader;
    GLuint shaderProgram;
    GLuint outputTexture;
    GLuint sceneDataSSBO;
    GLuint lightDataSSBO;
    GLuint materialDataSSBO;
    GLuint bvhDataSSBO;

    Scene* scene;
    Camera camera_;
    BVHNode bvh_;

public:
    PreferenceManager& prefs = PreferenceManager::getInstance();

    bool reflectionsEnabled;
    bool refractionsEnabled;
    bool specularEnabled;
    bool attenuationEnabled;
    bool shadowsEnabled;
    bool immediateEffect;
    bool fresnelEnabled;
    bool roughnessEnabled;
    bool bvhEnabled;
    bool oversamplingEnabled;
    int oversamplingFactor;
    int width, height;
    ComputeRenderer(Scene* scene, const Camera& camera, int width, int height);
    ~ComputeRenderer();

    bool initialize();
    void render(std::vector<Vector3>& frameBuffer);
    void setCamera(const Camera& camera);
    void cleanup();

private:
    bool loadComputeShader(const std::string& shaderSource);
    std::string loadShaderSource(const std::string& filename);
    std::string loadShaderWithIncludes(const std::string& filePath, const std::string& basePath, std::unordered_set<std::string>* includedFiles);
    void setupBuffers();
    void updateSceneData();
    void updateCameraUniforms() const;
};
