#include "StatisticsWindow.h"
#include "../Application.h"

StatisticsWindow::StatisticsWindow(Application& renderer)
    : UIWindow("Statistics"), m_renderer(renderer) {}

void StatisticsWindow::render() {
    if (!m_visible) return;
    
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowBgAlpha(0.8f);

    if (ImGui::Begin(m_name, nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        if (m_renderer.isRendering) {
            ImGui::Text("Rendering in progress...");
            ImGui::ProgressBar(-1.0f * (float)ImGui::GetTime(), ImVec2(200, 0), "");
        } else {
            ImGui::Text("Render Time: %.3f seconds", m_renderer.renderTime);
            ImGui::Text("Resolution: %dx%d", m_renderer.imageWidth, m_renderer.imageHeight);
            ImGui::Text("Total Pixels: %d", m_renderer.imageWidth * m_renderer.imageHeight);

            if (m_renderer.renderTime > 0) {
                const double pixelsPerSecond = (m_renderer.imageWidth * m_renderer.imageHeight) / m_renderer.renderTime;
                ImGui::Text("Pixels/second: %.0f", pixelsPerSecond);
            }
        }
    }
    ImGui::End();
}