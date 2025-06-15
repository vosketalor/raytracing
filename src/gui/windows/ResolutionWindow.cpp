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
                const bool isSelected = m_renderer.selectedPreset == i;
                if (ImGui::Selectable(m_renderer.resolutionPresets[i].name, isSelected)) {
                    m_renderer.selectedPreset = i;
                    if (i < m_renderer.resolutionPresets.size() - 1) {
                        m_renderer.renderWidth = m_renderer.resolutionPresets[i].width;
                        m_renderer.renderHeight = m_renderer.resolutionPresets[i].height;
                        m_renderer.customResolution = false;
                    } else {
                        m_renderer.customResolution = true;
                    }

                    auto& prefs = PreferenceManager::getInstance();
                    prefs.set("width", m_renderer.renderWidth);
                    prefs.set("height", m_renderer.renderHeight);
                    prefs.set("preset", m_renderer.selectedPreset);
                    prefs.save();
                }
                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        if (m_renderer.customResolution) {
            bool updated = false;
            updated |= ImGui::InputInt("Width", &m_renderer.renderWidth);
            updated |= ImGui::InputInt("Height", &m_renderer.renderHeight);

            if (updated) {
                auto& prefs = PreferenceManager::getInstance();
                prefs.set("width", m_renderer.renderWidth);
                prefs.set("height", m_renderer.renderHeight);
                prefs.save();
            }
        }

        ImGui::Text("Current: %dx%d", m_renderer.renderWidth, m_renderer.renderHeight);

        if (ImGui::Button("Apply & Render") && !m_renderer.isRendering) {
            m_renderer.triggerRerender();
        }
    }
    ImGui::End();
}