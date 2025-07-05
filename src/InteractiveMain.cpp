#include <iostream>
#include <fstream>
#include <chrono>
#include <memory>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <string>

// ImGui includes
#include <GL/glew.h>
#include <GLFW/glfw3.h>


#include "engine/Renderer.h"
#include "scenes/Scene1.h"

#include "Camera.h"
#include "imgui_impl_glfw.h"
#include "gui/Application.h"
#include "gui/UIManager.h"
#include "gui/InputManager.h"
#include "scenes/SceneFresnel.h"
#include "scenes/SceneGPU.h"
#include "scenes/SceneMicrofacets.h"

constexpr glm::vec3 SKYCOLOR = {135.0 / 255, 206.0 / 255, 235.0 / 255};
// constexpr glm::vec3 SKYCOLOR = Scene::BLACK;
static bool rightMousePressed = false;
static bool leftMousePressed = false;
static double lastMouseX = 0.f;
static double lastMouseY = 0.f;
static double mouseXf, mouseYf = 0.f;
constexpr float mouseSensitivity = 1.f;

#ifdef _WIN32
#include <windows.h>
extern "C" {
    __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

void performRender(Application &application, const int width, const int height)
{
    application.setRendering(true);
    application.renderer.width = width;
    application.renderer.height = height;

    const auto startTime = std::chrono::high_resolution_clock::now();

    application.renderer.setCamera(application.camera);
    std::vector<Vector3> frameBuffer(width * height);

    application.renderer.render(frameBuffer);

    const auto endTime = std::chrono::high_resolution_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count() / 1000.0;

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
        application.updateImage(frameBuffer, width, height);
    }

    application.setRenderTime(duration);
    application.setRendering(false);
}

static double lastFrameTime = 0.0;

static bool keysPressed[512] = {false};
static bool mouseCaptured = false;

void keyCallback(GLFWwindow *window, const int key, const int scancode, const int action, const int mods)
{
    if (mouseCaptured)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    else
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    if (action == GLFW_PRESS)
    {
        keysPressed[key] = true;
    }
    else if (action == GLFW_RELEASE)
    {
        keysPressed[key] = false;
    }
}



void mouseButtonCallback(GLFWwindow *window, const int button, const int action, const int mods)
{

    ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);

    if (button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        if (action == GLFW_PRESS)
        {
            rightMousePressed = true;
            mouseCaptured = true;
            glfwGetCursorPos(window, &lastMouseX, &lastMouseY);
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        else if (action == GLFW_RELEASE)
        {
            rightMousePressed = false;
            mouseCaptured = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS)
        {
            leftMousePressed = true;
        }
    }
}

void cursorPosCallback(GLFWwindow *window, const double xpos, const double ypos)
{

    ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);

    if (rightMousePressed)
    {
        double xoffset = xpos - lastMouseX;
        double yoffset = lastMouseY - ypos;
        lastMouseX = xpos;
        lastMouseY = ypos;

        xoffset *= mouseSensitivity;
        yoffset *= mouseSensitivity;

        Application *application = static_cast<Application *>(glfwGetWindowUserPointer(window));
        application->camera.processMouseMovement(static_cast<float>(xoffset), static_cast<float>(yoffset));
        application->triggerRerender();
    }
}

int main(const int argc, char *argv[])
{
    auto& prefs = PreferenceManager::getInstance();
    prefs.load();
    const int width = prefs.get("width", 512);
    const int height = prefs.get("height", 384);

    const auto scene = std::make_unique<SceneGPU>();
    scene->setSkyColor(SKYCOLOR);
    scene->setAmbient({0.3f, 0.3f, 0.3f});
    scene->createLights();
    scene->createShapes();

    Application application(scene.get());
    if (!application.initialize())
    {
        std::cerr << "Failed to initialize ImGui renderer" << std::endl;
        return 1;
    }

    UIManager uiManager(application);
    uiManager.initialize();

    std::cout << "Starting initial render..." << std::endl;
    std::cout << "Resolution: " << width << "x" << height << std::endl;
    std::cout << "GPU: " << glGetString(GL_RENDERER) << std::endl;

    glfwSetKeyCallback(application.window, keyCallback);
    glfwSetMouseButtonCallback(application.window, mouseButtonCallback);
    glfwSetCursorPosCallback(application.window, cursorPosCallback);
    glfwSetWindowUserPointer(application.window, &application);

    application.startAccumulation();
    performRender(application, width, height);

    while (!application.shouldClose())
    {
        const double currentFrameTime = glfwGetTime();
        const double deltaTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;

        if (keysPressed[GLFW_KEY_ESCAPE])
        {
            glfwSetWindowShouldClose(application.window, true);
        }
        if (keysPressed[GLFW_KEY_F11])
        {
            if (glfwGetWindowMonitor(application.window) == nullptr)
            {
                GLFWmonitor *monitor = glfwGetPrimaryMonitor();
                const GLFWvidmode *mode = glfwGetVideoMode(monitor);
                glfwSetWindowMonitor(application.window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
            }
            else
            {
                glfwSetWindowMonitor(application.window, nullptr, 100, 100, 1400, 900, 0);
            }
        }
        if (keysPressed[GLFW_KEY_W] || keysPressed[GLFW_KEY_Z])
        {
            application.camera.moveForward(static_cast<float>(deltaTime));
            application.triggerRerender();
        }
        if (keysPressed[GLFW_KEY_S])
        {
            application.camera.moveBackward(static_cast<float>(deltaTime));
            application.triggerRerender();
        }
        if (keysPressed[GLFW_KEY_A] || keysPressed[GLFW_KEY_Q])
        {
            application.camera.moveLeft(static_cast<float>(deltaTime));
            application.triggerRerender();
        }
        if (keysPressed[GLFW_KEY_D])
        {
            application.camera.moveRight(static_cast<float>(deltaTime));
            application.triggerRerender();
        }
        if (keysPressed[GLFW_KEY_SPACE])
        {
            application.camera.moveUp(static_cast<float>(deltaTime));
            application.triggerRerender();
        }
        if (keysPressed[GLFW_KEY_LEFT_SHIFT])
        {
            application.camera.moveDown(static_cast<float>(deltaTime));
            application.triggerRerender();
        }

        if (!ImGui::GetIO().WantCaptureMouse && rightMousePressed)
        {
            static glm::vec3 lastCamPos = application.camera.getPosition();
            static double lastCamPitch = application.camera.getPitch();
            static double lastCamYaw = application.camera.getYaw();

            if (lastCamPos != application.camera.getPosition() ||
            lastCamPitch != application.camera.getPitch() ||
            lastCamYaw != application.camera.getYaw())
            {
                application.triggerRerender();
                lastCamPos = application.camera.getPosition();
                lastCamPitch = application.camera.getPitch();
                lastCamYaw = application.camera.getYaw();
            }
        }

        if (leftMousePressed) {
            double mouseX, mouseY;
            glfwGetCursorPos(application.window, &mouseX, &mouseY);

            // 1. Conversion relative à la zone de rendu
            float relX = mouseX - application.imgScreenOrigin.x;
            float relY = mouseY - application.imgScreenOrigin.y;

            // 2. Normalisation [0-1]
            float normX = relX / application.imgScreenSize.x;
            float normY = relY / application.imgScreenSize.y;

            // 3. Conversion vers coordonnées texture
            int texX = static_cast<int>(normX * application.renderWidth);
            int texY = static_cast<int>(normY * application.renderHeight);

            const int shapeId = application.renderer.pick(texX, texY);
            if (shapeId >= 0 && shapeId < application.renderer.scene->getShapes().size())
            {
                Shape* selectedShape = application.renderer.scene->getShapes()[shapeId].get();

                if (application.lastShape && application.lastShape != selectedShape)
                {
                    application.lastShape->setWireframeEnabled(false); // Désactive l'ancien
                }

                if (application.lastShape == selectedShape)
                {
                    // On re-clique sur le même : toggle off
                    application.lastShape->setWireframeEnabled(false);
                    application.lastShape = nullptr;
                }
                else
                {
                    selectedShape->setWireframeEnabled(true);
                    application.lastShape = selectedShape;
                }
            } else
            {
                if (application.lastShape)
                {
                    application.lastShape->setWireframeEnabled(false);
                    application.lastShape = nullptr;
                }
            }

            application.renderer.updateSceneData();
            performRender(application, application.renderWidth, application.renderHeight);

            leftMousePressed = false;
        }
        if (application.needsRerender() || (application.needsContinuousRender() && !application.isRendering))
        {
            performRender(application, application.renderWidth, application.renderHeight);
        }

        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        uiManager.render();
        uiManager.update(static_cast<float>(deltaTime));

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(application.window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.0f, 0.0f, 0.0f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(application.window);
    }

    return 0;
}