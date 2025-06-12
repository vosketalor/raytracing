#pragma once

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>


#define IMGUI_IMPL_OPENGL_LOADER_GLEW
#include "imgui_impl_opengl3.h"   // uniquement ce fichier ImGui

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <GL/glext.h>

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