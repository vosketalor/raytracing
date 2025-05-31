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
#include "gui/ImGuiRenderer.h"

const Vector3 SKYCOLOR = {135.0 / 255, 206.0 / 255, 235.0 / 255};
static bool rightMousePressed = false;
static double lastMouseX = 0.0;
static double lastMouseY = 0.0;
const float mouseSensitivity = 1.0f;

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

    renderer.render(width, height, frameBuffer, guiRenderer.camera);

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

        ImGuiRenderer *guiRenderer = static_cast<ImGuiRenderer *>(glfwGetWindowUserPointer(window));
        guiRenderer->camera.processMouseMovement(xoffset, yoffset);
        guiRenderer->triggerRerender();
    }
}

int main(const int argc, char *argv[])
{
    int width = 512;
    int height = 384;

    ImGuiRenderer guiRenderer;
    if (!guiRenderer.initialize())
    {
        std::cerr << "Failed to initialize ImGui renderer" << std::endl;
        return 1;
    }

    std::cout << "Starting initial render..." << std::endl;
    std::cout << "Resolution: " << width << "x" << height << std::endl;

    glfwSetKeyCallback(guiRenderer.window, keyCallback);
    glfwSetMouseButtonCallback(guiRenderer.window, mouseButtonCallback);
    glfwSetCursorPosCallback(guiRenderer.window, cursorPosCallback);
    glfwSetWindowUserPointer(guiRenderer.window, &guiRenderer);

    guiRenderer.startAccumulation();
    performRender(guiRenderer, width, height);

    while (!guiRenderer.shouldClose())
    {
        double currentFrameTime = glfwGetTime();
        double deltaTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;

        if (keysPressed[GLFW_KEY_ESCAPE])
        {
            glfwSetWindowShouldClose(guiRenderer.window, true);
        }
        if (keysPressed[GLFW_KEY_F11])
        {
            if (glfwGetWindowMonitor(guiRenderer.window) == nullptr)
            {
                GLFWmonitor *monitor = glfwGetPrimaryMonitor();
                const GLFWvidmode *mode = glfwGetVideoMode(monitor);
                glfwSetWindowMonitor(guiRenderer.window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
            }
            else
            {
                glfwSetWindowMonitor(guiRenderer.window, nullptr, 100, 100, 1400, 900, 0);
            }
        }
        if (keysPressed[GLFW_KEY_W] || keysPressed[GLFW_KEY_Z])
        {
            guiRenderer.camera.moveForward(deltaTime);
            guiRenderer.triggerRerender();
        }
        if (keysPressed[GLFW_KEY_S])
        {
            guiRenderer.camera.moveBackward(deltaTime);
            guiRenderer.triggerRerender();
        }
        if (keysPressed[GLFW_KEY_A] || keysPressed[GLFW_KEY_Q])
        {
            guiRenderer.camera.moveLeft(deltaTime);
            guiRenderer.triggerRerender();
        }
        if (keysPressed[GLFW_KEY_D])
        {
            guiRenderer.camera.moveRight(deltaTime);
            guiRenderer.triggerRerender();
        }
        if (keysPressed[GLFW_KEY_SPACE])
        {
            guiRenderer.camera.moveUp(deltaTime);
            guiRenderer.triggerRerender();
        }
        if (keysPressed[GLFW_KEY_LEFT_SHIFT])
        {
            guiRenderer.camera.moveDown(deltaTime);
            guiRenderer.triggerRerender();
        }

        if (!ImGui::GetIO().WantCaptureMouse && rightMousePressed)
        {
            static Vector3 lastCamPos = guiRenderer.camera.getPosition();
            static double lastCamPitch = guiRenderer.camera.getPitch();
            static double lastCamYaw = guiRenderer.camera.getYaw();

            if (lastCamPos != guiRenderer.camera.getPosition() ||
                lastCamPitch != guiRenderer.camera.getPitch() ||
                lastCamYaw != guiRenderer.camera.getYaw())
            {
                guiRenderer.triggerRerender();
                lastCamPos = guiRenderer.camera.getPosition();
                lastCamPitch = guiRenderer.camera.getPitch();
                lastCamYaw = guiRenderer.camera.getYaw();
            }
        }

        if (guiRenderer.needsRerender() || (guiRenderer.needsContinuousRender() && !guiRenderer.isRendering))
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
            ImGui::ProgressBar(-1.0f * (float)ImGui::GetTime(), ImVec2(200, 0), "");
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

        ImGui::Separator();

        ImGui::Text("Noise Reduction");
        const bool accumulationChanged = ImGui::Checkbox("Enable Accumulation", &guiRenderer.enableAccumulation);

        if (guiRenderer.enableAccumulation)
        {
            ImGui::SliderInt("Max Samples", &guiRenderer.maxSamples, 4, 64);
        }

        if (accumulationChanged && !guiRenderer.enableAccumulation)
        {
            guiRenderer.resetAccumulation();
        }

        if (accumulationChanged && guiRenderer.enableAccumulation)
        {
            guiRenderer.triggerRerender();
        }

        if (guiRenderer.enableAccumulation)
        {
            ImGui::Separator();
            ImGui::Text("Accumulation: %d/%d samples", guiRenderer.currentSample, guiRenderer.maxSamples);
            if (guiRenderer.accumulationInProgress)
            {
                const float progress = static_cast<float>(guiRenderer.currentSample) / static_cast<float>(guiRenderer.maxSamples);
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