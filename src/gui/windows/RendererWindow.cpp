#include "RendererWindow.h"
#include "../Application.h"

RendererWindow::RendererWindow(Application& renderer)
    : UIWindow("Renderer Settings"), m_renderer(renderer) {}

void reflection(Application& m_renderer) {
    if (ImGui::Checkbox("Enable Reflection", &m_renderer.renderer.reflectionsEnabled)) {
        m_renderer.renderer.prefs.set("reflectionsEnabled", m_renderer.renderer.reflectionsEnabled);
        m_renderer.renderer.prefs.save();

        if (!m_renderer.renderer.reflectionsEnabled) {
            m_renderer.renderer.fresnelEnabled = false;
            m_renderer.renderer.prefs.set("fresnelEnabled", m_renderer.renderer.fresnelEnabled);
            m_renderer.renderer.prefs.save();
        }

        if (!m_renderer.isRendering && m_renderer.renderer.immediateEffect) m_renderer.triggerRerender();
    }
}

void refraction(Application& m_renderer) {
    if (ImGui::Checkbox("Enable Refraction", &m_renderer.renderer.refractionsEnabled)) {
        m_renderer.renderer.prefs.set("refractionsEnabled", m_renderer.renderer.refractionsEnabled);
        m_renderer.renderer.prefs.save();

        if (!m_renderer.renderer.refractionsEnabled) {
            m_renderer.renderer.fresnelEnabled = false;
            m_renderer.renderer.prefs.set("fresnelEnabled", m_renderer.renderer.fresnelEnabled);
            m_renderer.renderer.prefs.save();
        }

        if (!m_renderer.isRendering && m_renderer.renderer.immediateEffect) m_renderer.triggerRerender();
    }
}

void specular(Application& m_renderer) {
    if (ImGui::Checkbox("Enable Specular", &m_renderer.renderer.specularEnabled)) {
        m_renderer.renderer.prefs.set("specularEnabled", m_renderer.renderer.specularEnabled);
        m_renderer.renderer.prefs.save();
        if (!m_renderer.isRendering && m_renderer.renderer.immediateEffect) m_renderer.triggerRerender();
    }
}

void attenuation(Application& m_renderer) {
    if (ImGui::Checkbox("Enable Attenuation", &m_renderer.renderer.attenuationEnabled)) {
        m_renderer.renderer.prefs.set("attenuationEnabled", m_renderer.renderer.attenuationEnabled);
        m_renderer.renderer.prefs.save();
        if (!m_renderer.isRendering && m_renderer.renderer.immediateEffect) m_renderer.triggerRerender();
    }
}

void fresnel(Application& m_renderer) {
    if (ImGui::Checkbox("Enable Fresnel", &m_renderer.renderer.fresnelEnabled)) {
        m_renderer.renderer.prefs.set("fresnelEnabled", m_renderer.renderer.fresnelEnabled);
        m_renderer.renderer.prefs.save();

        if (m_renderer.renderer.fresnelEnabled)
        {
            m_renderer.renderer.reflectionsEnabled = true;
            m_renderer.renderer.refractionsEnabled = true;
            m_renderer.renderer.prefs.set("reflectionsEnabled", m_renderer.renderer.reflectionsEnabled);
            m_renderer.renderer.prefs.set("refractionsEnabled", m_renderer.renderer.refractionsEnabled);
            m_renderer.renderer.prefs.save();
        }
        if (!m_renderer.isRendering && m_renderer.renderer.immediateEffect) m_renderer.triggerRerender();
    }
}

void bvh(Application& m_renderer) {
    if (ImGui::Checkbox("Enable BVH", &m_renderer.renderer.bvhEnabled)) {
        m_renderer.renderer.prefs.set("bvhEnabled", m_renderer.renderer.bvhEnabled);
        m_renderer.renderer.prefs.save();
        if (!m_renderer.isRendering && m_renderer.renderer.immediateEffect) m_renderer.triggerRerender();
    }
}

void roughness(Application& m_renderer) {
    if (ImGui::Checkbox("Enable Roughness", &m_renderer.renderer.roughnessEnabled)) {
        m_renderer.renderer.prefs.set("roughnessEnabled", m_renderer.renderer.roughnessEnabled);
        m_renderer.renderer.prefs.save();
        if (!m_renderer.isRendering && m_renderer.renderer.immediateEffect) m_renderer.triggerRerender();
    }
}

void shadow(Application& m_renderer) {
    if (ImGui::Checkbox("Enable Shadows", &m_renderer.renderer.shadowsEnabled)) {
        m_renderer.renderer.prefs.set("shadowsEnabled", m_renderer.renderer.shadowsEnabled);
        m_renderer.renderer.prefs.save();
        if (!m_renderer.isRendering) m_renderer.triggerRerender();
    }

    // if (m_renderer.renderer.shadowsEnabled) {
    //     ImGui::Checkbox("Enable Soft Shadows", &m_renderer.renderer.softShadowsEnabled);
    //     if (m_renderer.renderer.softShadowsEnabled) {
    //         ImGui::SliderInt("Samples", &m_renderer.renderer.samplesNumber, 1, 32);
    //     }
    // }
}

void immediateEffect(Application& m_renderer) {
    if (ImGui::Checkbox("Immediate effect", &m_renderer.renderer.immediateEffect)) {
        m_renderer.renderer.prefs.set("immediateEffect", m_renderer.renderer.immediateEffect);
        m_renderer.renderer.prefs.save();
        if (!m_renderer.isRendering) m_renderer.triggerRerender();
    }
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

        fresnel(m_renderer);

        ImGui::Separator();

        specular(m_renderer);

        roughness(m_renderer);

        attenuation(m_renderer);

        ImGui::Separator();

        shadow(m_renderer);

        ImGui::Separator();

        bvh(m_renderer);

        accumulation(m_renderer);

        ImGui::Separator();

        immediateEffect(m_renderer);

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