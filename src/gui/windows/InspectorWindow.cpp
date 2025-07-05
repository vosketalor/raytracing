#include "InspectorWindow.h"
#include "../Application.h"
#include "glm/gtc/type_ptr.hpp"
#include "shapes/Sphere.h"
#include "shapes/Triangle.h"

InspectorWindow::InspectorWindow(Application& renderer)
    : UIWindow("Inspector"), m_renderer(renderer) {}

void InspectorWindow::renderShape(Application& renderer)
{
    if (typeid(*renderer.lastShape) == typeid(Sphere)) {
        Sphere* sphere = static_cast<Sphere*>(renderer.lastShape);

        ImGui::InputFloat3("Center", glm::value_ptr(sphere->center));

        ImGui::InputFloat("Radius", &sphere->radius);

        // Optionnel : mettre à jour la bounding box après modification
        if (ImGui::IsItemDeactivatedAfterEdit()) {
            sphere->setBoundingBox();
        }
    } else if (typeid(*renderer.lastShape) == typeid(Plane) || typeid(*renderer.lastShape) == typeid(Triangle)) {
        // Plane ou Triangle
        auto* plane = static_cast<Plane*>(renderer.lastShape);

        ImGui::InputFloat3("Normal", glm::value_ptr(plane->normal));
        ImGui::InputFloat("Distance to origin", &plane->distance);

        bool modified = ImGui::IsItemDeactivatedAfterEdit();

        // Vérifie si c'est un Triangle
        if (Triangle* triangle = dynamic_cast<Triangle*>(renderer.lastShape)) {
            ImGui::Separator();
            modified |= ImGui::InputFloat3("A", glm::value_ptr(triangle->A));
            modified |= ImGui::InputFloat3("B", glm::value_ptr(triangle->B));
            modified |= ImGui::InputFloat3("C", glm::value_ptr(triangle->C));
        }

        if (modified) {
            plane->setBoundingBox();
        }
    }
}


void InspectorWindow::render() {
    if (!m_visible || !m_renderer.enabledWindows.rendererWindow || m_renderer.lastShape == nullptr) return;

    const ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(10, io.DisplaySize.y / 2), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowBgAlpha(0.9f);

    if (ImGui::Begin(m_name, nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        if (ImGui::BeginTabBar("InspectorTabs", ImGuiTabBarFlags_None)) {

            // Onglet "Shape"
            if (ImGui::BeginTabItem("Shape")) {
                renderShape(m_renderer);
                ImGui::EndTabItem();
            }

            // Onglet "Material" (exemple vide à remplir plus tard)
            if (ImGui::BeginTabItem("Material")) {
                ImGui::Text("Material properties here...");
                ImGui::EndTabItem();
            }

            // Onglet "Transform" (autre exemple possible)
            if (ImGui::BeginTabItem("Transform")) {
                ImGui::Text("Transform tools here...");
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }
    }

    ImGui::End();
}
