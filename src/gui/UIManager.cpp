#include "UIManager.h"
#include "windows/StatisticsWindow.h"
#include "windows/ResolutionWindow.h"
#include "windows/MainViewport.h"
#include "windows/RendererWindow.h"

#include "Application.h"
#include <algorithm>

#include "windows/MenuBar.h"


UIManager::UIManager(Application& renderer) : m_renderer(renderer) {}

void UIManager::initialize() {
    addWindow(std::make_unique<MainViewport>(m_renderer));
    addWindow(std::make_unique<StatisticsWindow>(m_renderer));
    addWindow(std::make_unique<ResolutionWindow>(m_renderer));
    addWindow(std::make_unique<RendererWindow>(m_renderer));
    addWindow(std::make_unique<MenuBar>(m_renderer));
}

void UIManager::render() {
    for (auto& window : m_windows) {
        window->render();
    }
}

void UIManager::update(float deltaTime) {
    for (auto& window : m_windows) {
        window->update(deltaTime);
    }
}

void UIManager::addWindow(std::unique_ptr<UIWindow> window) {
    m_windows.push_back(std::move(window));
}

void UIManager::removeWindow(const char* name) {
    auto it = std::remove_if(m_windows.begin(), m_windows.end(),
        [name](const std::unique_ptr<UIWindow>& window) {
            return strcmp(window->getName(), name) == 0;
        });
    m_windows.erase(it, m_windows.end());
}

UIWindow* UIManager::getWindow(const char* name) {
    auto it = std::find_if(m_windows.begin(), m_windows.end(),
        [name](const std::unique_ptr<UIWindow>& window) {
            return strcmp(window->getName(), name) == 0;
        });
    return it != m_windows.end() ? it->get() : nullptr;
}

void UIManager::setWindowVisible(const char* name, bool visible) {
    if (auto* window = getWindow(name)) {
        window->setVisible(visible);
    }
}