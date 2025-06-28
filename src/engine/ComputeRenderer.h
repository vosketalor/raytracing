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
    GLuint pickShader;
    GLuint shaderProgram;
    GLuint pickShaderProgram;
    GLuint outputTexture;
    GLuint sceneDataSSBO;
    GLuint lightDataSSBO;
    GLuint materialDataSSBO;
    GLuint bvhDataSSBO;
    GLuint pickSSBO;
    GLuint textureAtlas;
    GLuint textureSSBO;

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
    Scene* scene;
    ComputeRenderer(Scene* scene, const Camera& camera, int width, int height);
    ~ComputeRenderer();

    bool initialize();
    void render(std::vector<Vector3>& frameBuffer);
    void createFallbackTexture();
    void setCamera(const Camera& camera);
    void cleanup();
    int pick(int mouseX, int mouseY) const;
    void updateSceneData();
    void updateUniforms() const;
    void updateCameraUniforms(const GLuint& program) const;

private:
    bool loadComputeShader(GLuint& shader, GLuint& program, const std::string& shaderSource);
    bool loadPickShader(const std::string& shaderSource);
    std::string loadShaderSource(const std::string& filename);
    std::string loadShaderWithIncludes(const std::string& filePath, const std::string& basePath, std::unordered_set<std::string>* includedFiles);
    void setupBuffers();
    void setUniform3f(GLuint program, const char* name, float x, float y, float z) const;
    void setUniform2f(GLuint program, const char* name, float x, float y) const;
    void setUniform1f(GLuint program, const char* name, float value) const;
    void setUniform1i(GLuint program, const char* name, int value) const;
    void setUniform2i(GLuint program, const char* name, int x, int y) const;
    void setUniformBool(GLuint program, const char* name, bool value) const;
};
