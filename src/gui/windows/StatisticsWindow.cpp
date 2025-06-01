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

        ImGui::Separator();

        ImGui::Text("Noise Reduction");
        const bool accumulationChanged = ImGui::Checkbox("Enable Accumulation", &m_renderer.enableAccumulation);

        if (m_renderer.enableAccumulation) {
            ImGui::SliderInt("Max Samples", &m_renderer.maxSamples, 4, 64);
        }

        if (accumulationChanged && !m_renderer.enableAccumulation) {
            m_renderer.resetAccumulation();
        }

        if (accumulationChanged && m_renderer.enableAccumulation) {
            m_renderer.triggerRerender();
        }

        if (m_renderer.enableAccumulation) {
            ImGui::Separator();
            ImGui::Text("Accumulation: %d/%d samples", m_renderer.currentSample, m_renderer.maxSamples);
            if (m_renderer.accumulationInProgress) {
                const float progress = static_cast<float>(m_renderer.currentSample) / static_cast<float>(m_renderer.maxSamples);
                ImGui::ProgressBar(progress, ImVec2(200, 0));
            }
        }

        ImGui::Separator();

        if (ImGui::Button("Save Image") && m_renderer.imageReady && !m_renderer.isRendering) {
            m_renderer.saveImage();
        }
        ImGui::SameLine();
        if (ImGui::Button("Re-render") && !m_renderer.isRendering) {
            m_renderer.triggerRerender();
        }
    }
    ImGui::End();
}