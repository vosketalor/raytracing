#include <iostream>
#include <fstream>
#include <chrono>
#include <memory>
#include <vector>
#include <algorithm>
#include <iomanip>

// ImGui includes
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// OpenGL includes
#include <GLFW/glfw3.h>

#include "engine/Renderer.h"
#include "scenes/Scene1.h"
#include <string>

#include "Camera.h"

const Vector3 SKYCOLOR = {135.0 / 255, 206.0 / 255, 235.0 / 255};

class ImGuiRenderer
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

    int renderWidth = 512;
    int renderHeight = 384;
    bool showResolutionMenu = false;

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

    bool enableAccumulation = true;
    int maxSamples = 16;
    int currentSample = 0;
    std::vector<Vector3> accumBuffer;
    std::vector<Vector3> currentBuffer;
    bool accumulationInProgress = false;

private:
    bool shouldRerender;

public:
    ImGuiRenderer() : window(nullptr), textureID(0), imageWidth(0), imageHeight(0),
                      renderTime(0.0), imageReady(false), isRendering(false), prevWindowWidth(-1),
                      prevWindowHeight(-1), shouldRerender(false) {}

    ~ImGuiRenderer()
    {
        cleanup();
    }

    bool initialize(const int windowWidth = 1400, const int windowHeight = 900)
    {
        // Initialisation GLFW
        if (!glfwInit())
        {
            std::cerr << "Failed to initialize GLFW" << std::endl;
            return false;
        }

        // Configuration OpenGL
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        // Création de la fenêtre
        window = glfwCreateWindow(windowWidth, windowHeight, "Ray Tracer Viewer", nullptr, nullptr);
        if (!window)
        {
            std::cerr << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            return false;
        }

        glfwMakeContextCurrent(window);
        glfwSwapInterval(1); // Enable vsync

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();

        // Setup Platform/Renderer backends
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 330");

        // Générer la texture pour l'image
        glGenTextures(1, &textureID);

        return true;
    }

    void updateImage(const std::vector<Vector3> &frameBuffer, int width, int height)
    {
        imageWidth = width;
        imageHeight = height;
        imageData.resize(width * height * 3);

        // Conversion du frameBuffer en données RGB 8-bit
        for (int i = 0; i < width * height; ++i)
        {
            Vector3 color = frameBuffer[i];
            color.clamp(0.0, 1.0);

            imageData[i * 3 + 0] = static_cast<unsigned char>(color[0] * 255);
            imageData[i * 3 + 1] = static_cast<unsigned char>(color[1] * 255);
            imageData[i * 3 + 2] = static_cast<unsigned char>(color[2] * 255);
        }

        // Upload texture to GPU
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, imageData.data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);

        imageReady = true;
    }

    void accumulateSample(const std::vector<Vector3> &newSample, int width, int height)
    {
        if (accumBuffer.empty())
        {
            accumBuffer.resize(width * height, Vector3(0, 0, 0));
            currentSample = 0;
        }

        currentSample++;

        for (int i = 0; i < width * height; ++i)
        {
            accumBuffer[i] = accumBuffer[i] + newSample[i];
        }

        std::vector<Vector3> displayBuffer(width * height);
        for (int i = 0; i < width * height; ++i)
        {
            displayBuffer[i] = accumBuffer[i] * (1.0 / currentSample);
        }

        updateImage(displayBuffer, width, height);
    }

    void resetAccumulation()
    {
        accumBuffer.clear();
        currentBuffer.clear();
        currentSample = 0;
        accumulationInProgress = false;
    }

    void setRenderTime(double time)
    {
        renderTime = time;
    }

    bool needsRerender() const
    {
        return shouldRerender;
    }

    bool needsContinuousRender()
    {
        return enableAccumulation && accumulationInProgress && currentSample < maxSamples;
    }

    void setRendering(const bool rendering)
    {
        isRendering = rendering;
        if (rendering)
        {
            shouldRerender = false;
        }
    }

    void startAccumulation()
    {
        if (enableAccumulation)
        {
            resetAccumulation();
            accumulationInProgress = true;
        }
    }

    bool shouldClose() const
    {
        return glfwWindowShouldClose(window);
    }

    void saveImage() const
    {
        if (!imageReady)
            return;

        const std::string filename = "output_" + std::to_string(std::time(nullptr)) + ".ppm";
        std::ofstream image(filename);
        if (!image.is_open())
        {
            std::cerr << "Error while opening file: " << filename << std::endl;
            return;
        }

        image << "P3\n"
              << imageWidth << " " << imageHeight << "\n255\n";

        for (int y = 0; y < imageHeight; ++y)
        {
            for (int x = 0; x < imageWidth; ++x)
            {
                const int idx = (y * imageWidth + x) * 3;
                const int r = imageData[idx + 0];
                const int g = imageData[idx + 1];
                const int b = imageData[idx + 2];
                image << r << ' ' << g << ' ' << b << '\n';
            }
        }

        image.close();
        std::cout << "Image saved: " << filename << std::endl;
    }

    void triggerRerender()
    {
        this->shouldRerender = true;
        if (enableAccumulation)
        {
            startAccumulation();
        }
    }

    void cleanup() const
    {
        if (textureID)
        {
            glDeleteTextures(1, &textureID);
        }

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        if (window)
        {
            glfwDestroyWindow(window);
        }
        glfwTerminate();
    }
};

void performRender(ImGuiRenderer &guiRenderer, const int width, const int height)
{
    guiRenderer.setRendering(true);

    const auto startTime = std::chrono::high_resolution_clock::now();

    const auto scene = std::make_unique<Scene1>();
    scene->setSkyColor(SKYCOLOR);
    scene->setAmbient({0.1f, 0.1f, 0.1f});
    scene->createLights();
    scene->createShapes();

    Renderer renderer(scene.get());
    std::vector<Vector3> frameBuffer(width * height);

    const Camera camera;

    renderer.render(width, height, frameBuffer, camera);

    const auto endTime = std::chrono::high_resolution_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count() / 1000.0;

    if (guiRenderer.enableAccumulation && guiRenderer.accumulationInProgress)
    {
        guiRenderer.accumulateSample(frameBuffer, width, height);

        if (guiRenderer.currentSample >= guiRenderer.maxSamples)
        {
            guiRenderer.accumulationInProgress = false;
        }
    }
    else
    {
        guiRenderer.updateImage(frameBuffer, width, height);
    }

    guiRenderer.setRenderTime(duration);
    guiRenderer.setRendering(false);
}

int main(const int argc, char *argv[])
{
    int width = 512;
    int height = 384;

    if (argc >= 3)
    {
        width = std::atoi(argv[1]);
        height = std::atoi(argv[2]);
    }

    ImGuiRenderer guiRenderer;
    if (!guiRenderer.initialize())
    {
        std::cerr << "Failed to initialize ImGui renderer" << std::endl;
        return 1;
    }

    std::cout << "Starting initial render..." << std::endl;
    std::cout << "Resolution: " << width << "x" << height << std::endl;

    guiRenderer.startAccumulation();
    performRender(guiRenderer, width, height);

    while (!guiRenderer.shouldClose())
    {
        if (guiRenderer.needsRerender())
        {
            performRender(guiRenderer, guiRenderer.renderWidth, guiRenderer.renderHeight);
        }

        if (guiRenderer.needsContinuousRender() && !guiRenderer.isRendering)
        {
            performRender(guiRenderer, guiRenderer.renderWidth, guiRenderer.renderHeight);
        }

        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (guiRenderer.imageReady)
        {
            const ImGuiIO &io = ImGui::GetIO();
            const float availableWidth = io.DisplaySize.x;
            const float availableHeight = io.DisplaySize.y;

            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSize(ImVec2(availableWidth, availableHeight));

            ImGui::Begin("Rendered Image", nullptr,
                         ImGuiWindowFlags_NoTitleBar |
                             ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoScrollbar |
                             ImGuiWindowFlags_NoCollapse |
                             ImGuiWindowFlags_NoBringToFrontOnFocus);

            const float imageRatio = static_cast<float>(guiRenderer.imageWidth) / static_cast<float>(guiRenderer.imageHeight);
            const float windowRatio = availableWidth / availableHeight;

            ImVec2 imageSize;
            ImVec2 imagePos;

            if (imageRatio > windowRatio)
            {
                imageSize.x = availableWidth;
                imageSize.y = availableWidth / imageRatio;
                imagePos.x = 0;
                imagePos.y = (availableHeight - imageSize.y) * 0.5f;
            }
            else
            {
                imageSize.x = availableHeight * imageRatio;
                imageSize.y = availableHeight;
                imagePos.x = (availableWidth - imageSize.x) * 0.5f;
                imagePos.y = 0;
            }

            ImDrawList *draw_list = ImGui::GetWindowDrawList();
            draw_list->AddRectFilled(ImVec2(0, 0), ImVec2(availableWidth, availableHeight),
                                     IM_COL32(0, 0, 0, 255));

            ImGui::SetCursorPos(imagePos);
            ImGui::Image(static_cast<ImTextureID>(static_cast<intptr_t>(guiRenderer.textureID)), imageSize);

            ImGui::End();
        }

        ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowBgAlpha(0.8f);

        ImGui::Begin("Statistics", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

        if (guiRenderer.isRendering)
        {
            ImGui::Text("Rendering in progress...");
            ImGui::ProgressBar(-1.0f * ImGui::GetTime(), ImVec2(200, 0), "");
        }
        else
        {
            ImGui::Text("Render Time: %.3f seconds", guiRenderer.renderTime);
            ImGui::Text("Resolution: %dx%d", guiRenderer.imageWidth, guiRenderer.imageHeight);
            ImGui::Text("Total Pixels: %d", guiRenderer.imageWidth * guiRenderer.imageHeight);

            if (guiRenderer.renderTime > 0)
            {
                const double pixelsPerSecond = (guiRenderer.imageWidth * guiRenderer.imageHeight) / guiRenderer.renderTime;
                ImGui::Text("Pixels/second: %.0f", pixelsPerSecond);
            }
        }

        if (guiRenderer.enableAccumulation)
        {
            ImGui::Separator();
            ImGui::Text("Accumulation: %d/%d samples", guiRenderer.currentSample, guiRenderer.maxSamples);
            if (guiRenderer.accumulationInProgress)
            {
                const float progress = static_cast<float>(guiRenderer.currentSample) / (float)guiRenderer.maxSamples;
                ImGui::ProgressBar(progress, ImVec2(200, 0));
            }
        }

        ImGui::Separator();

        if (ImGui::Button("Save Image") && guiRenderer.imageReady && !guiRenderer.isRendering)
        {
            guiRenderer.saveImage();
        }
        ImGui::SameLine();
        if (ImGui::Button("Re-render") && !guiRenderer.isRendering)
        {
            guiRenderer.triggerRerender();
        }

        ImGui::End();

        const ImGuiIO &io = ImGui::GetIO();
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - 320, 10), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowBgAlpha(0.9f);

        ImGui::Begin("Render Settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

        ImGui::Text("Noise Reduction");
        ImGui::Separator();

        const bool accumulationChanged = ImGui::Checkbox("Enable Accumulation", &guiRenderer.enableAccumulation);

        if (guiRenderer.enableAccumulation)
        {
            ImGui::SliderInt("Max Samples", &guiRenderer.maxSamples, 4, 64);

            if (ImGui::Button("Reset Accumulation"))
            {
                guiRenderer.resetAccumulation();
            }
        }

        if (accumulationChanged && !guiRenderer.enableAccumulation)
        {
            guiRenderer.resetAccumulation();
        }

        ImGui::End();

        // Menu Resolution
        ImGui::SetNextWindowPos(ImVec2(10, io.DisplaySize.y - 210), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowBgAlpha(0.9f);

        ImGui::Begin("Resolution Settings", nullptr,
                     ImGuiWindowFlags_AlwaysAutoResize);

        ImGui::Text("Render Resolution");
        ImGui::Separator();

        if (ImGui::BeginCombo("Preset", guiRenderer.resolutionPresets[guiRenderer.selectedPreset].name))
        {
            for (int i = 0; i < guiRenderer.resolutionPresets.size(); i++)
            {
                const bool isSelected = (guiRenderer.selectedPreset == i);
                if (ImGui::Selectable(guiRenderer.resolutionPresets[i].name, isSelected))
                {
                    guiRenderer.selectedPreset = i;
                    if (i < guiRenderer.resolutionPresets.size() - 1)
                    {
                        guiRenderer.renderWidth = guiRenderer.resolutionPresets[i].width;
                        guiRenderer.renderHeight = guiRenderer.resolutionPresets[i].height;
                        guiRenderer.customResolution = false;
                    }
                    else
                    {
                        guiRenderer.customResolution = true;
                    }
                }
                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        if (guiRenderer.customResolution)
        {
            ImGui::InputInt("Width", &guiRenderer.renderWidth);
            ImGui::InputInt("Height", &guiRenderer.renderHeight);
        }

        ImGui::Text("Current: %dx%d", guiRenderer.renderWidth, guiRenderer.renderHeight);

        if (ImGui::Button("Apply & Render") && !guiRenderer.isRendering)
        {
            guiRenderer.triggerRerender();
        }

        ImGui::End();

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(guiRenderer.window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.0f, 0.0f, 0.0f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(guiRenderer.window);
    }

    return 0;
}