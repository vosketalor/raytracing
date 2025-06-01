#include "InputManager.h"
#include "Window.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include <GLFW/glfw3.h>

const float MOUSE_SENSITIVITY = 1.0f;

InputManager::InputManager(Window& window) : m_window(window) {
    // Setup callbacks
    m_window.setKeyCallback([this](int key, int scancode, int action, int mods) {
        onKey(key, scancode, action, mods);
    });

    m_window.setMouseButtonCallback([this](int button, int action, int mods) {
        onMouseButton(button, action, mods);
    });

    m_window.setCursorPosCallback([this](double xpos, double ypos) {
        onCursorPos(xpos, ypos);
    });
}

void InputManager::update(float deltaTime) {
    processInput(deltaTime);
}

void InputManager::processInput(float deltaTime) {
    // Process individual key callbacks
    for (const auto& [key, callback] : m_keyCallbacks) {
        if (isKeyPressed(key) && callback) {
            callback(key, GLFW_PRESS, 0);
        }
    }
}

bool InputManager::isKeyPressed(int key) const {
    auto it = m_keysPressed.find(key);
    return it != m_keysPressed.end() && it->second;
}

void InputManager::setKeyPressed(int key, bool pressed) {
    m_keysPressed[key] = pressed;
}

void InputManager::setKeyCallback(int key, const KeyCallback& callback) {
    m_keyCallbacks[key] = callback;
}

void InputManager::onKey(int key, int scancode, int action, int mods) {
    // Forward to ImGui first - check if ImGui wants to capture keyboard
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureKeyboard) {
        return; // Let ImGui handle it
    }

    // Update cursor mode based on mouse capture state
    if (m_mouseState.captured) {
        m_window.setCursorMode(GLFW_CURSOR_DISABLED);
    } else {
        m_window.setCursorMode(GLFW_CURSOR_NORMAL);
    }

    if (action == GLFW_PRESS) {
        setKeyPressed(key, true);
    } else if (action == GLFW_RELEASE) {
        setKeyPressed(key, false);
    }
}

void InputManager::onMouseButton(int button, int action, int mods) {
    // Forward to ImGui first - check if ImGui wants to capture mouse
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) {
        ImGui_ImplGlfw_MouseButtonCallback(m_window.getHandle(), button, action, mods);
        return; // Let ImGui handle it exclusively
    }

    // Forward to ImGui anyway for proper state tracking
    ImGui_ImplGlfw_MouseButtonCallback(m_window.getHandle(), button, action, mods);

    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            m_mouseState.rightPressed = true;
            m_mouseState.captured = true;
            m_window.getCursorPos(m_mouseState.lastX, m_mouseState.lastY);
            m_window.setCursorMode(GLFW_CURSOR_DISABLED);
        } else if (action == GLFW_RELEASE) {
            m_mouseState.rightPressed = false;
            m_mouseState.captured = false;
            m_window.setCursorMode(GLFW_CURSOR_NORMAL);
        }
    } else if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            m_mouseState.leftHandled = false;
            m_window.getCursorPos(m_mouseState.lastX, m_mouseState.lastY);
        }
    }
}

void InputManager::onCursorPos(double xpos, double ypos) {
    // Forward to ImGui first
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) {
        ImGui_ImplGlfw_CursorPosCallback(m_window.getHandle(), xpos, ypos);
        return; // Let ImGui handle it exclusively
    }

    // Forward to ImGui anyway for proper state tracking
    ImGui_ImplGlfw_CursorPosCallback(m_window.getHandle(), xpos, ypos);

    if (m_mouseState.rightPressed && m_mouseState.captured) {
        double xoffset = xpos - m_mouseState.lastX;
        double yoffset = m_mouseState.lastY - ypos;

        m_mouseState.lastX = xpos;
        m_mouseState.lastY = ypos;

        xoffset *= MOUSE_SENSITIVITY;
        yoffset *= MOUSE_SENSITIVITY;

        // TODO: Apply mouse movement to camera
        // This is where you would update camera rotation based on mouse movement
        // Example: camera.rotate(xoffset, yoffset);
    }
}