#pragma once
#include <GLFW/glfw3.h>
#include <functional>
#include <string>

class Window {
public:
    struct WindowConfig {
        int width = 1400;
        int height = 900;
        std::string title = "Raytracer";
        bool vsync = true;
        bool resizable = true;
    };

    // Callbacks types
    using KeyCallback = std::function<void(int key, int scancode, int action, int mods)>;
    using MouseButtonCallback = std::function<void(int button, int action, int mods)>;
    using CursorPosCallback = std::function<void(double xpos, double ypos)>;
    using FramebufferSizeCallback = std::function<void(int width, int height)>;
    using WindowSizeCallback = std::function<void(int width, int height)>;

    Window() = default;
    ~Window();

    bool initialize(const WindowConfig& config = WindowConfig{});
    void shutdown();

    bool shouldClose() const;
    void setShouldClose(bool close);
    void swapBuffers();
    void pollEvents();

    // Fullscreen management
    void toggleFullscreen();
    bool isFullscreen() const;

    // Window properties
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    GLFWwindow* getHandle() const { return m_window; }
    
    void getFramebufferSize(int& width, int& height) const;

    // Cursor management
    void setCursorMode(int mode);
    void getCursorPos(double& xpos, double& ypos) const;

    // Callbacks
    void setKeyCallback(const KeyCallback& callback);
    void setMouseButtonCallback(const MouseButtonCallback& callback);
    void setCursorPosCallback(const CursorPosCallback& callback);
    void setFramebufferSizeCallback(const FramebufferSizeCallback& callback);
    void setWindowSizeCallback(const WindowSizeCallback& callback);

    // User pointer for custom data
    void setUserPointer(void* pointer);
    void* getUserPointer() const;

private:
    GLFWwindow* m_window = nullptr;
    int m_width = 0;
    int m_height = 0;
    int m_windowedWidth = 0;
    int m_windowedHeight = 0;
    int m_windowedPosX = 0;
    int m_windowedPosY = 0;
    bool m_isFullscreen = false;
    std::string m_title;

    // Callbacks
    KeyCallback m_keyCallback;
    MouseButtonCallback m_mouseButtonCallback;
    CursorPosCallback m_cursorPosCallback;
    FramebufferSizeCallback m_framebufferSizeCallback;
    WindowSizeCallback m_windowSizeCallback;

    // Static GLFW callbacks
    static void glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void glfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void glfwCursorPosCallback(GLFWwindow* window, double xpos, double ypos);
    static void glfwFramebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void glfwWindowSizeCallback(GLFWwindow* window, int width, int height);
    static void glfwErrorCallback(int error, const char* description);

    bool initializeGLFW();
    void setupCallbacks();
};