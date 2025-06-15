#include "RendererWindow.h"
#include "../Application.h"

RendererWindow::RendererWindow(Application& renderer)
    : UIWindow("Renderer Settings"), m_renderer(renderer) {}

void reflection(Application& m_renderer) {
    ImGui::Checkbox("Enable reflection", &m_renderer.renderer.reflectionsEnabled);
}

void refraction(Application& m_renderer) {
    ImGui::Checkbox("Enable refraction", &m_renderer.renderer.refractionsEnabled);
}

void specular(Application& m_renderer) {
    ImGui::Checkbox("Enable specular", &m_renderer.renderer.specularEnabled);
}

void attenuation(Application& m_renderer) {
    ImGui::Checkbox("Enable attenuation", &m_renderer.renderer.attenuationEnabled);
}

void shadow(Application& m_renderer) {
    ImGui::Checkbox("Enable Shadows", &m_renderer.renderer.shadowsEnabled);
    // if (m_renderer.renderer.shadowsEnabled) {
    //     ImGui::Checkbox("Enable Soft Shadows", &m_renderer.renderer.softShadowsEnabled);
    //     if (m_renderer.renderer.softShadowsEnabled) {
    //         ImGui::SliderInt("Samples", &m_renderer.renderer.samplesNumber, 1, 32);
    //     }
    // }
}

void accumulation(Application& m_renderer) {
    const bool accumulationChanged = ImGui::Checkbox("Enable Noise Reduction", &m_renderer.enableAccumulation);

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
}

void RendererWindow::render() {
    if (!m_visible || !m_renderer.enabledWindows.rendererWindow) return;

    const ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(10, io.DisplaySize.y / 2), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowBgAlpha(0.9f);

    if (ImGui::Begin(m_name, nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {

        reflection(m_renderer);

        refraction(m_renderer);

        specular(m_renderer);

        attenuation(m_renderer);

        ImGui::Separator();

        shadow(m_renderer);

        ImGui::Separator();

        accumulation(m_renderer);

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