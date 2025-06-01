#pragma once
#include <vector>
#include <memory>
#include "windows/UIWindow.h"

class Application;

class UIManager {
public:
    UIManager(Application& renderer);
    ~UIManager() = default;

    void initialize();
    void render();
    void update(float deltaTime);
    
    void addWindow(std::unique_ptr<UIWindow> window);
    void removeWindow(const char* name);
    UIWindow* getWindow(const char* name);
    
    void setWindowVisible(const char* name, bool visible);

private:
    Application& m_renderer;
    std::vector<std::unique_ptr<UIWindow>> m_windows;
};