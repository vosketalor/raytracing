#include "Application.h"
#include <ctime>

bool Application::initialize(const int windowWidth, const int windowHeight)
{
    // Configuration de la fenêtre
    WindowConfig config;
    config.width = windowWidth;
    config.height = windowHeight;
    config.title = "Ray Tracer";
    config.resizable = true;
    config.vsync = true;

    // Initialize window with config (ceci initialise déjà GLFW)
    if (!window.initialize(config)) {
        std::cerr << "Failed to initialize window" << std::endl;
        return false;
    }

    // Vérifier que le contexte OpenGL est actif
    if (!glfwGetCurrentContext()) {
        std::cerr << "No OpenGL context available" << std::endl;
        return false;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    if (!ImGui_ImplGlfw_InitForOpenGL(window.getHandle(), true)) {
        std::cerr << "Failed to initialize ImGui GLFW backend" << std::endl;
        return false;
    }

    if (!ImGui_ImplOpenGL3_Init("#version 330")) {
        std::cerr << "Failed to initialize ImGui OpenGL3 backend" << std::endl;
        return false;
    }

    // Générer la texture pour l'image APRÈS l'initialisation d'OpenGL
    glGenTextures(1, &textureID);
    if (glGetError() != GL_NO_ERROR) {
        std::cerr << "Failed to generate texture" << std::endl;
        return false;
    }

    return true;
}

void Application::updateImage(const std::vector<Vector3> &frameBuffer, int width, int height)
{
    if (frameBuffer.empty() || width <= 0 || height <= 0) {
        std::cerr << "Invalid framebuffer data" << std::endl;
        return;
    }

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

    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL error in updateImage: " << error << std::endl;
        return;
    }

    imageReady = true;
}

void Application::accumulateSample(const std::vector<Vector3> &newSample, const int width, const int height)
{
    if (newSample.empty() || width <= 0 || height <= 0) {
        std::cerr << "Invalid sample data" << std::endl;
        return;
    }

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
    return window.shouldClose();
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

    // La fenêtre se détruira elle-même via son destructeur
}

void Application::handleMouse() {
    InputManager::MouseState& mouseState = inputManager.getMouseState();
    if (mouseState.rightPressed)
    {
        static Vector3 lastCamPos = camera.getPosition();
        static double lastCamPitch = camera.getPitch();
        static double lastCamYaw = camera.getYaw();

        if (lastCamPos != camera.getPosition() ||
            lastCamPitch != camera.getPitch() ||
            lastCamYaw != camera.getYaw())
        {
            triggerRerender();
            lastCamPos = camera.getPosition();
            lastCamPitch = camera.getPitch();
            lastCamYaw = camera.getYaw();
        }
    }
}

void Application::handleInput(float deltaTime) {
    inputManager.update(deltaTime);

    if (inputManager.isKeyPressed(GLFW_KEY_ESCAPE)) {
        window.setShouldClose(true);
    }
    if (inputManager.isKeyPressed(GLFW_KEY_F11)) {
        window.toggleFullscreen();
    }
    if (inputManager.isKeyPressed(GLFW_KEY_W) || inputManager.isKeyPressed(GLFW_KEY_Z)) {
        camera.moveForward(deltaTime);
        triggerRerender();
    }
    if (inputManager.isKeyPressed(GLFW_KEY_S)) {
        camera.moveBackward(deltaTime);
        triggerRerender();
    }
    if (inputManager.isKeyPressed(GLFW_KEY_A) || inputManager.isKeyPressed(GLFW_KEY_Q)) {
        camera.moveLeft(deltaTime);
        triggerRerender();
    }
    if (inputManager.isKeyPressed(GLFW_KEY_D)) {
        camera.moveRight(deltaTime);
        triggerRerender();
    }
    if (inputManager.isKeyPressed(GLFW_KEY_SPACE)) {
        camera.moveUp(deltaTime);
        triggerRerender();
    }
    if (inputManager.isKeyPressed(GLFW_KEY_LEFT_SHIFT)) {
        camera.moveDown(deltaTime);
        triggerRerender();
    }
}