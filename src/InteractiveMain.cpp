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

const Vector3 SKYCOLOR = {135.0 / 255, 206.0 / 255, 235.0 / 255};
static bool rightMousePressed = false;
static double lastMouseX = 0.0;
static double lastMouseY = 0.0;
const float mouseSensitivity = 1.0f;

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

void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
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

void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
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
}

void cursorPosCallback(GLFWwindow *window, double xpos, double ypos)
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
        application->camera.processMouseMovement(xoffset, yoffset);
        application->triggerRerender();
    }
}

int main(const int argc, char *argv[])
{
    int width = 512;
    int height = 384;

    const auto scene = std::make_unique<SceneGPU>();
    scene->setSkyColor(SKYCOLOR);
    scene->setAmbient({0.1f, 0.1f, 0.1f});
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

    glfwSetKeyCallback(application.window, keyCallback);
    glfwSetMouseButtonCallback(application.window, mouseButtonCallback);
    glfwSetCursorPosCallback(application.window, cursorPosCallback);
    glfwSetWindowUserPointer(application.window, &application);

    application.startAccumulation();
    performRender(application, width, height);

    while (!application.shouldClose())
    {
        double currentFrameTime = glfwGetTime();
        double deltaTime = currentFrameTime - lastFrameTime;
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
            application.camera.moveForward(deltaTime);
            application.triggerRerender();
        }
        if (keysPressed[GLFW_KEY_S])
        {
            application.camera.moveBackward(deltaTime);
            application.triggerRerender();
        }
        if (keysPressed[GLFW_KEY_A] || keysPressed[GLFW_KEY_Q])
        {
            application.camera.moveLeft(deltaTime);
            application.triggerRerender();
        }
        if (keysPressed[GLFW_KEY_D])
        {
            application.camera.moveRight(deltaTime);
            application.triggerRerender();
        }
        if (keysPressed[GLFW_KEY_SPACE])
        {
            application.camera.moveUp(deltaTime);
            application.triggerRerender();
        }
        if (keysPressed[GLFW_KEY_LEFT_SHIFT])
        {
            application.camera.moveDown(deltaTime);
            application.triggerRerender();
        }

        if (!ImGui::GetIO().WantCaptureMouse && rightMousePressed)
        {
            static Vector3 lastCamPos = application.camera.getPosition();
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