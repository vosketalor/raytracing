#pragma once

// ImGui includes
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// OpenGL includes
#include <GLFW/glfw3.h>

#include <vector>
#include <iostream>
#include <fstream>
#include <string>

#include "Renderer.h"
#include "../engine/Camera.h"
#include "InputManager.h"
#include "Window.h"

class Application
{
public:
    Window window;
    GLuint textureID;
    int imageWidth, imageHeight;
    std::vector<unsigned char> imageData;
    double renderTime;
    bool imageReady;
    bool isRendering;
    int prevWindowWidth = -1;
    int prevWindowHeight = -1;

    int renderWidth = 512;
    int renderHeight = 384;
    bool showResolutionMenu = false;

    Camera camera;
    Renderer renderer;

    struct ResolutionPreset
    {
        const char *name;
        int width;
        int height;
    };

    std::vector<ResolutionPreset> resolutionPresets = {
        {"512x384", 512, 384},
        {"640x480", 640, 480},
        {"800x600", 800, 600},
        {"1024x768", 1024, 768},
        {"1280x720", 1280, 720},
        {"1920x1080", 1920, 1080},
        {"Custom", 0, 0}};

    int selectedPreset = 0;
    bool customResolution = false;

    bool enableAccumulation = false;
    int maxSamples = 16;
    int currentSample = 0;
    std::vector<Vector3> accumBuffer;
    std::vector<Vector3> currentBuffer;
    bool accumulationInProgress = false;
    bool shouldRerender;

    InputManager inputManager; // Ajouter InputManager


private:
public:
    Application(Scene* scene) : window(), textureID(0), imageWidth(0), imageHeight(0),
                  renderTime(0.0), imageReady(false), isRendering(false), prevWindowWidth(-1),
                  prevWindowHeight(-1), renderer(scene, camera, 0, 0), shouldRerender(false),
                  inputManager(window) {}

    ~Application()
    {
        cleanup();
    }

    bool initialize(const int windowWidth = 1400, const int windowHeight = 900);
    void updateImage(const std::vector<Vector3> &frameBuffer, int width, int height);
    void accumulateSample(const std::vector<Vector3> &newSample, const int width, const int height);
    void resetAccumulation();
    void setRenderTime(const double time);
    bool needsRerender() const;
    bool needsContinuousRender() const;
    void setRendering(const bool rendering);
    void startAccumulation();
    bool shouldClose() const;
    void saveImage() const;
    void triggerRerender();
    void cleanup() const;
    void handleInput(float deltaTime);
    void handleMouse();
};
