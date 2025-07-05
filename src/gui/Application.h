#pragma once

struct GLFWwindow;  // forward declaration

// autres includes locaux seulement
#include <vector>
#include "Renderer.h"
#include "ComputeRenderer.h"
#include "../engine/Camera.h"


class Application
{
public:
    GLFWwindow *window;
    GLuint textureID;
    int imageWidth, imageHeight;
    std::vector<unsigned char> imageData;
    double renderTime;
    bool imageReady;
    bool isRendering;
    int prevWindowWidth = -1;
    int prevWindowHeight = -1;

    int renderWidth;
    int renderHeight;
    bool showResolutionMenu = false;

    bool softShadowsEnabled = false;

    struct EnabledWindows {
        bool rendererWindow = true;
        bool resolutionWindow = true;
        bool statisticsWindow = true;
    };

    EnabledWindows enabledWindows;

    Camera camera;
    ComputeRenderer renderer;

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

    int selectedPreset;
    bool customResolution = false;

    bool enableAccumulation = false;
    int maxSamples = 16;
    int currentSample = 0;
    std::vector<Vector3> accumBuffer;
    std::vector<Vector3> currentBuffer;
    bool accumulationInProgress = false;
    bool shouldRerender;

    ImVec2 imgScreenOrigin;  // coin haut‑gauche de l'image sur l'écran (pixels GLFW/OS)
    ImVec2 imgScreenSize;    // taille de l'image à l'écran

    Shape* lastShape = nullptr;

private:
public:
    Application(Scene* scene) : window(nullptr), textureID(0), imageWidth(0), imageHeight(0),
                      renderTime(0.0), imageReady(false), isRendering(false), prevWindowWidth(-1),
                      prevWindowHeight(-1), renderer(scene, camera, 0, 0), shouldRerender(false) {
        auto& prefs = PreferenceManager::getInstance();
        prefs.load();
        renderWidth = prefs.get("width", 512);
        renderHeight = prefs.get("height", 384);
        selectedPreset = prefs.get("preset", 0);
    }

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
};