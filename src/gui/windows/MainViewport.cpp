#include "MainViewport.h"
#include "../Application.h"

MainViewport::MainViewport(Application& renderer)
    : UIWindow("Main Viewport"), m_renderer(renderer) {}

void MainViewport::render() {
    if (!m_visible || !m_renderer.imageReady) return;
    
    const ImGuiIO& io = ImGui::GetIO();
    const float availableWidth = io.DisplaySize.x;
    const float availableHeight = io.DisplaySize.y;

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(availableWidth, availableHeight));

    if (ImGui::Begin("Rendered Image", nullptr,
                     ImGuiWindowFlags_NoTitleBar |
                     ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoScrollbar |
                     ImGuiWindowFlags_NoScrollWithMouse |
                     ImGuiWindowFlags_NoCollapse |
                     ImGuiWindowFlags_NoBringToFrontOnFocus)) {

        const float imageRatio = static_cast<float>(m_renderer.imageWidth) / static_cast<float>(m_renderer.imageHeight);
        const float windowRatio = availableWidth / availableHeight;

        ImVec2 imageSize;
        ImVec2 imagePos;

        if (imageRatio > windowRatio) {
            imageSize.x = availableWidth;
            imageSize.y = availableWidth / imageRatio;
            imagePos.x = 0;
            imagePos.y = (availableHeight - imageSize.y) * 0.5f;
        } else {
            imageSize.x = availableHeight * imageRatio;
            imageSize.y = availableHeight;
            imagePos.x = (availableWidth - imageSize.x) * 0.5f;
            imagePos.y = 0;
        }

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        draw_list->AddRectFilled(ImVec2(0, 0), ImVec2(availableWidth, availableHeight),
                                 IM_COL32(0, 0, 0, 255));

        ImGui::SetCursorPos(imagePos);
        ImGui::Image(static_cast<ImTextureID>(static_cast<intptr_t>(m_renderer.textureID)), imageSize);

        const ImVec2 winPos = ImGui::GetWindowPos();
        m_renderer.imgScreenOrigin.x = winPos.x + imagePos.x;
        m_renderer.imgScreenOrigin.y = winPos.y + imagePos.y;
        m_renderer.imgScreenSize    = imageSize;
    }
    ImGui::End();
}