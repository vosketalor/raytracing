#include <GL/glew.h>        // doit être en premier, avant glfw
#include <GLFW/glfw3.h>

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "Application.h"
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>

bool Application::initialize(const int windowWidth, const int windowHeight)
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
    glfwSwapInterval(1);

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

    renderer.initialize();

    return true;
}

void Application::updateImage(const std::vector<Vector3> &frameBuffer, int width, int height)
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

void Application::accumulateSample(const std::vector<Vector3> &newSample, const int width, const int height)
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

void Application::resetAccumulation()
{
    accumBuffer.clear();
    currentBuffer.clear();
    currentSample = 0;
    accumulationInProgress = false;
}

void Application::setRenderTime(const double time)
{
    renderTime = time;
}

bool Application::needsRerender() const
{
    return shouldRerender;
}

bool Application::needsContinuousRender() const
{
    return enableAccumulation && accumulationInProgress && currentSample < maxSamples;
}

void Application::setRendering(const bool rendering)
{
    isRendering = rendering;
    if (rendering)
    {
        shouldRerender = false;
    }
}

void Application::startAccumulation()
{
    if (enableAccumulation)
    {
        resetAccumulation();
        accumulationInProgress = true;
    }
}

bool Application::shouldClose() const
{
    return glfwWindowShouldClose(window);
}

void Application::saveImage() const
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

void Application::triggerRerender()
{
    this->shouldRerender = true;
    if (enableAccumulation)
    {
        startAccumulation();
    }
}

void Application::cleanup() const
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