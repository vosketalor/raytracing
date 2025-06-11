#include "ResolutionWindow.h"
#include "../Application.h"

ResolutionWindow::ResolutionWindow(Application& renderer)
    : UIWindow("Resolution Settings"), m_renderer(renderer) {}

void ResolutionWindow::render() {
    if (!m_visible || !m_renderer.enabledWindows.resolutionWindow) return;
    
    const ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(10, io.DisplaySize.y - 210), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowBgAlpha(0.9f);

    if (ImGui::Begin(m_name, nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Render Resolution");
        ImGui::Separator();

        if (ImGui::BeginCombo("Preset", m_renderer.resolutionPresets[m_renderer.selectedPreset].name)) {
            for (int i = 0; i < m_renderer.resolutionPresets.size(); i++) {
                const bool isSelected = (m_renderer.selectedPreset == i);
                if (ImGui::Selectable(m_renderer.resolutionPresets[i].name, isSelected)) {
                    m_renderer.selectedPreset = i;
                    if (i < m_renderer.resolutionPresets.size() - 1) {
                        m_renderer.renderWidth = m_renderer.resolutionPresets[i].width;
                        m_renderer.renderHeight = m_renderer.resolutionPresets[i].height;
                        m_renderer.customResolution = false;
                    } else {
                        m_renderer.customResolution = true;
                    }
                }
                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        if (m_renderer.customResolution) {
            ImGui::InputInt("Width", &m_renderer.renderWidth);
            ImGui::InputInt("Height", &m_renderer.renderHeight);
        }

        ImGui::Text("Current: %dx%d", m_renderer.renderWidth, m_renderer.renderHeight);

        if (ImGui::Button("Apply & Render") && !m_renderer.isRendering) {
            m_renderer.triggerRerender();
        }
    }
    ImGui::End();
}