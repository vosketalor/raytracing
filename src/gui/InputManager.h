#pragma once
#include <unordered_map>
#include <functional>

class Window;

class InputManager {
public:
    using KeyCallback = std::function<void(int key, int action, int mods)>;
    
    struct MouseState {
        bool rightPressed = false;
        double lastX = 0.0;
        double lastY = 0.0;
        bool captured = false;
    };
    
    InputManager(Window& window);
    
    void update(float deltaTime);
    void processInput(float deltaTime);
    
    bool isKeyPressed(int key) const;
    void setKeyPressed(int key, bool pressed);
    
    const MouseState& getMouseState() const { return m_mouseState; }
    MouseState& getMouseState() { return m_mouseState; }
    
    void setKeyCallback(int key, const KeyCallback& callback);
    
private:
    Window& m_window;
    std::unordered_map<int, bool> m_keysPressed;
    MouseState m_mouseState;
    std::unordered_map<int, KeyCallback> m_keyCallbacks;
    
    void onKey(int key, int scancode, int action, int mods);
    void onMouseButton(int button, int action, int mods);
    void onCursorPos(double xpos, double ypos);
};