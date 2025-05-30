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

const Vector3 SKYCOLOR = {135.0/255, 206.0/255, 235.0/255};

class ImGuiRenderer {
public:
    GLFWwindow* window;
    GLuint textureID;
    int imageWidth, imageHeight;
    std::vector<unsigned char> imageData;
    double renderTime;
    bool imageReady;
    bool isRendering;

private:
    bool shouldRerender;

public:
    ImGuiRenderer() : window(nullptr), textureID(0), imageWidth(0), imageHeight(0), 
                      renderTime(0.0), imageReady(false), shouldRerender(false), isRendering(false) {}

    ~ImGuiRenderer() {
        cleanup();
    }

    bool initialize(int windowWidth = 1400, int windowHeight = 900) {
        // Initialisation GLFW
        if (!glfwInit()) {
            std::cerr << "Failed to initialize GLFW" << std::endl;
            return false;
        }

        // Configuration OpenGL
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        // Création de la fenêtre
        window = glfwCreateWindow(windowWidth, windowHeight, "Ray Tracer Viewer", nullptr, nullptr);
        if (!window) {
            std::cerr << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            return false;
        }

        glfwMakeContextCurrent(window);
        glfwSwapInterval(1); // Enable vsync

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
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

    void updateImage(const std::vector<Vector3>& frameBuffer, int width, int height) {
        imageWidth = width;
        imageHeight = height;
        imageData.resize(width * height * 3);

        // Conversion du frameBuffer en données RGB 8-bit
        for (int i = 0; i < width * height; ++i) {
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

    void setRenderTime(double time) {
        renderTime = time;
    }

    bool needsRerender() {
        return shouldRerender;
    }

    void setRendering(bool rendering) {
        isRendering = rendering;
        if (rendering) {
            shouldRerender = false;
        }
    }

    bool shouldClose() {
        return glfwWindowShouldClose(window);
    }

    void saveImage() {
        if (!imageReady) return;

        std::string filename = "output_" + std::to_string(std::time(nullptr)) + ".ppm";
        std::ofstream image(filename);
        if (!image.is_open()) {
            std::cerr << "Error while opening file: " << filename << std::endl;
            return;
        }

        image << "P3\n" << imageWidth << " " << imageHeight << "\n255\n";

        for (int y = 0; y < imageHeight; ++y) {
            for (int x = 0; x < imageWidth; ++x) {
                int idx = (y * imageWidth + x) * 3;
                int r = imageData[idx + 0];
                int g = imageData[idx + 1];
                int b = imageData[idx + 2];
                image << r << ' ' << g << ' ' << b << '\n';
            }
        }

        image.close();
        std::cout << "Image saved: " << filename << std::endl;
    }

    void triggerRerender() {
        this->shouldRerender = true;
    }

    void cleanup() {
        if (textureID) {
            glDeleteTextures(1, &textureID);
        }
        
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        if (window) {
            glfwDestroyWindow(window);
        }
        glfwTerminate();
    }
};

void performRender(ImGuiRenderer& guiRenderer, int width, int height) {
    guiRenderer.setRendering(true);
    
    auto startTime = std::chrono::high_resolution_clock::now();

    auto scene = std::make_unique<Scene1>();
    scene->setSkyColor(SKYCOLOR);
    scene->setAmbient({0.1f, 0.1f, 0.1f});
    scene->createLights();
    scene->createShapes();

    Renderer renderer(scene.get());
    std::vector<Vector3> frameBuffer(width * height);

    renderer.render(width, height, frameBuffer);

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count() / 1000.0;

    std::cout << "Rendering completed in " << duration << " seconds" << std::endl;

    guiRenderer.updateImage(frameBuffer, width, height);
    guiRenderer.setRenderTime(duration);
    guiRenderer.setRendering(false);
}

int main(const int argc, char *argv[]) {
    int width = 512;
    int height = 384;
    
    if (argc >= 3) {
        width = std::atoi(argv[1]);
        height = std::atoi(argv[2]);
    }

    ImGuiRenderer guiRenderer;
    if (!guiRenderer.initialize()) {
        std::cerr << "Failed to initialize ImGui renderer" << std::endl;
        return 1;
    }

    std::cout << "Starting initial render..." << std::endl;
    std::cout << "Resolution: " << width << "x" << height << std::endl;

    performRender(guiRenderer, width, height);

    while (!guiRenderer.shouldClose()) {
        // Vérifier si un nouveau rendu est demandé
        if (guiRenderer.needsRerender()) {
            std::cout << "Starting re-render..." << std::endl;
            performRender(guiRenderer, width, height);
        }
        
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Obtenir la taille de la fenêtre
        int windowWidth, windowHeight;
        glfwGetWindowSize(guiRenderer.window, &windowWidth, &windowHeight);

        // Fenêtre pour afficher l'image en plein écran
        if (guiRenderer.imageReady) {
            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSize(ImVec2(windowWidth, windowHeight));
            
            ImGui::Begin("Rendered Image", nullptr, 
                ImGuiWindowFlags_NoTitleBar | 
                ImGuiWindowFlags_NoResize | 
                ImGuiWindowFlags_NoMove | 
                ImGuiWindowFlags_NoScrollbar |
                ImGuiWindowFlags_NoCollapse |
                ImGuiWindowFlags_NoBringToFrontOnFocus);
            
            // Calcul de la taille pour remplir la fenêtre en conservant le ratio
            float aspectRatio = static_cast<float>(guiRenderer.imageWidth) / static_cast<float>(guiRenderer.imageHeight);
            float windowAspectRatio = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
            
            float displayWidth, displayHeight;
            float offsetX = 0, offsetY = 0;
            
            if (aspectRatio > windowAspectRatio) {
                displayWidth = windowWidth;
                displayHeight = windowWidth / aspectRatio;
                offsetY = (windowHeight - displayHeight) / 2;
            } else {
                displayHeight = windowHeight;
                displayWidth = windowHeight * aspectRatio;
                offsetX = (windowWidth - displayWidth) / 2;
            }
            
            ImGui::SetCursorPos(ImVec2(offsetX, offsetY));
            ImGui::Image((ImTextureID)(intptr_t)guiRenderer.textureID, ImVec2(displayWidth, displayHeight));
            
            ImGui::End();
        }

        // Fenêtre overlay pour les statistiques
        ImGui::SetNextWindowPos(ImVec2(10, 10));
        ImGui::SetNextWindowBgAlpha(0.8f);
        
        ImGui::Begin("Statistics", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        
        if (guiRenderer.isRendering) {
            ImGui::Text("Rendering in progress...");
            ImGui::ProgressBar(-1.0f * ImGui::GetTime(), ImVec2(200, 0), "");
        } else {
            ImGui::Text("Render Time: %.3f seconds", guiRenderer.renderTime);
            ImGui::Text("Resolution: %dx%d", guiRenderer.imageWidth, guiRenderer.imageHeight);
            ImGui::Text("Total Pixels: %d", guiRenderer.imageWidth * guiRenderer.imageHeight);
            
            if (guiRenderer.renderTime > 0) {
                double pixelsPerSecond = (guiRenderer.imageWidth * guiRenderer.imageHeight) / guiRenderer.renderTime;
                ImGui::Text("Pixels/second: %.0f", pixelsPerSecond);
            }
        }
        
        ImGui::Separator();
        
        if (ImGui::Button("Save Image") && guiRenderer.imageReady && !guiRenderer.isRendering) {
            guiRenderer.saveImage();
        }
        ImGui::SameLine();
        if (ImGui::Button("Re-render") && !guiRenderer.isRendering) {
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