#include "MenuBar.h"
#include "../Application.h"

MenuBar::MenuBar(Application& renderer)
    : UIWindow("MenuBar"), m_renderer(renderer) {}

void MenuBar::render() {
    if (!m_visible) return;

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Exit")) {
                glfwSetWindowShouldClose(m_renderer.window, true);
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Menu")) {
            ImGui::Checkbox("Statistics", &m_renderer.enabledWindows.statisticsWindow);
            ImGui::Checkbox("Renderer", &m_renderer.enabledWindows.rendererWindow);
            ImGui::Checkbox("Resolution", &m_renderer.enabledWindows.resolutionWindow);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Scenes")) {
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}