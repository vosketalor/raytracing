#include "ImGuiRenderer.h"

#include <ctime>

bool ImGuiRenderer::initialize(const int windowWidth, const int windowHeight)
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

    return true;
}

void ImGuiRenderer::updateImage(const std::vector<Vector3> &frameBuffer, int width, int height)
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

void ImGuiRenderer::accumulateSample(const std::vector<Vector3> &newSample, const int width, const int height)
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

void ImGuiRenderer::resetAccumulation()
{
    accumBuffer.clear();
    currentBuffer.clear();
    currentSample = 0;
    accumulationInProgress = false;
}

void ImGuiRenderer::setRenderTime(const double time)
{
    renderTime = time;
}

bool ImGuiRenderer::needsRerender() const
{
    return shouldRerender;
}

bool ImGuiRenderer::needsContinuousRender() const
{
    return enableAccumulation && accumulationInProgress && currentSample < maxSamples;
}

void ImGuiRenderer::setRendering(const bool rendering)
{
    isRendering = rendering;
    if (rendering)
    {
        shouldRerender = false;
    }
}

void ImGuiRenderer::startAccumulation()
{
    if (enableAccumulation)
    {
        resetAccumulation();
        accumulationInProgress = true;
    }
}

bool ImGuiRenderer::shouldClose() const
{
    return glfwWindowShouldClose(window);
}

void ImGuiRenderer::saveImage() const
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

void ImGuiRenderer::triggerRerender()
{
    this->shouldRerender = true;
    if (enableAccumulation)
    {
        startAccumulation();
    }
}

void ImGuiRenderer::cleanup() const
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