#pragma once

// ImGui includes
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// OpenGL includes
#include <GLFW/glfw3.h>

class UIWindow {
public:
    UIWindow(const char* name, bool* open = nullptr) 
        : m_name(name), m_open(open), m_visible(true) {}
    
    virtual ~UIWindow() = default;
    
    virtual void render() = 0;
    virtual void update(float deltaTime) {}
    
    bool isVisible() const { return m_visible; }
    void setVisible(bool visible) { m_visible = visible; }
    
    const char* getName() const { return m_name; }

protected:
    const char* m_name;
    bool* m_open;
    bool m_visible;
};