#include <iostream>
#include <fstream>
#include <chrono>
#include <memory>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <string>

// ImGui includes
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// OpenGL includes
#include <GLFW/glfw3.h>

#include "engine/Renderer.h"
#include "scenes/Scene1.h"

#include "Camera.h"
#include "gui/Application.h"
#include "gui/UIManager.h"
#include "gui/InputManager.h"

const Vector3 SKYCOLOR = {135.0 / 255, 206.0 / 255, 235.0 / 255};

void performRender(Application &application, const int width, const int height)
{
    std::cout << "DEBUG: performRender() called with " << width << "x" << height << std::endl;

    // Vérifications de sécurité
    if (width <= 0 || height <= 0) {
        std::cerr << "ERROR: Invalid dimensions in performRender: " << width << "x" << height << std::endl;
        return;
    }

    std::cout << "DEBUG: Setting rendering state..." << std::endl;
    application.setRendering(true);
    application.renderer.width = width;
    application.renderer.height = height;

    const auto startTime = std::chrono::high_resolution_clock::now();

    std::cout << "DEBUG: Setting camera..." << std::endl;
    application.renderer.setCamera(application.camera);

    std::cout << "DEBUG: Creating framebuffer with size: " << (width * height) << std::endl;
    std::vector<Vector3> frameBuffer;
    try {
        frameBuffer.resize(width * height);
        std::cout << "DEBUG: Framebuffer created successfully" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "ERROR: Failed to create framebuffer: " << e.what() << std::endl;
        application.setRendering(false);
        return;
    }

    std::cout << "DEBUG: Starting render..." << std::endl;
    try {
        application.renderer.render(frameBuffer);
        std::cout << "DEBUG: Render completed successfully" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "ERROR: Render failed: " << e.what() << std::endl;
        application.setRendering(false);
        return;
    }

    const auto endTime = std::chrono::high_resolution_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count() / 1000.0;

    std::cout << "DEBUG: Processing accumulation..." << std::endl;
    if (application.enableAccumulation && application.accumulationInProgress)
    {
        application.accumulateSample(frameBuffer, width, height);

        if (application.currentSample >= application.maxSamples)
        {
            application.accumulationInProgress = false;
        }
    }
    else
    {
        std::cout << "DEBUG: Updating image..." << std::endl;
        application.updateImage(frameBuffer, width, height);
    }

    application.setRenderTime(duration);
    application.setRendering(false);
    std::cout << "DEBUG: performRender() completed" << std::endl;
}

static double lastFrameTime = 0.0;

int main(const int argc, char *argv[])
{
    std::cout << "DEBUG: Starting main()" << std::endl;

    int width = 512;
    int height = 384;

    std::cout << "DEBUG: Creating scene..." << std::endl;
    const auto scene = std::make_unique<Scene1>();
    if (!scene) {
        std::cerr << "ERROR: Failed to create scene" << std::endl;
        return 1;
    }

    std::cout << "DEBUG: Configuring scene..." << std::endl;
    try {
        scene->setSkyColor(SKYCOLOR);
        scene->setAmbient({0.1f, 0.1f, 0.1f});
        std::cout << "DEBUG: Creating lights..." << std::endl;
        scene->createLights();
        std::cout << "DEBUG: Creating shapes..." << std::endl;
        scene->createShapes();
        std::cout << "DEBUG: Scene configuration completed" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "ERROR: Scene configuration failed: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "DEBUG: Creating application..." << std::endl;
    Application application(scene.get());

    std::cout << "DEBUG: Initializing application..." << std::endl;
    if (!application.initialize())
    {
        std::cerr << "ERROR: Failed to initialize application" << std::endl;
        return 1;
    }
    std::cout << "DEBUG: Application initialized successfully" << std::endl;

    std::cout << "DEBUG: Creating UI manager..." << std::endl;
    UIManager uiManager(application);
    try {
        uiManager.initialize();
        std::cout << "DEBUG: UI manager initialized successfully" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "ERROR: UI manager initialization failed: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "Starting initial render..." << std::endl;
    std::cout << "Resolution: " << width << "x" << height << std::endl;

    std::cout << "DEBUG: Starting accumulation..." << std::endl;
    application.startAccumulation();

    std::cout << "DEBUG: About to call performRender()..." << std::endl;
    std::flush(std::cout); // Force l'affichage avant le crash potentiel

    try {
        performRender(application, width, height);
        std::cout << "DEBUG: Initial render completed successfully" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "ERROR: Initial render failed: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "DEBUG: Entering main loop..." << std::endl;
    while (!application.shouldClose())
    {
        // Poll events first
        glfwPollEvents();

        double currentFrameTime = glfwGetTime();
        double deltaTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;

        try {
            // Start the Dear ImGui frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            // Handle input only if ImGui doesn't want to capture it
            if (!ImGui::GetIO().WantCaptureKeyboard) {
                application.handleInput(static_cast<float>(deltaTime));
            }

            if (!ImGui::GetIO().WantCaptureMouse) {
                application.handleMouse();
            }

            // Check if we need to render
            if (application.needsRerender() || (application.needsContinuousRender() && !application.isRendering)) {
                performRender(application, application.renderWidth, application.renderHeight);
            }

            // Update and render UI
            uiManager.render();
            uiManager.update(static_cast<float>(deltaTime));

            // Rendering
            ImGui::Render();
            int display_w, display_h;
            glfwGetFramebufferSize(application.window.getHandle(), &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            glClearColor(0.0f, 0.0f, 0.0f, 1.00f);
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            application.window.swapBuffers();

        } catch (const std::exception& e) {
            std::cerr << "ERROR in main loop: " << e.what() << std::endl;
            break;
        }
    }

    std::cout << "DEBUG: Exiting main()" << std::endl;
    return 0;
}