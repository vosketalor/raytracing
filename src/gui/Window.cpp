#include "Window.h"
#include <iostream>

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

Window::~Window() {
    shutdown();
}

bool Window::initialize(const WindowConfig& config) {
    m_width = config.width;
    m_height = config.height;
    m_title = config.title;
    m_windowedWidth = config.width;
    m_windowedHeight = config.height;

    if (!initializeGLFW()) {
        return false;
    }

    // Set window hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, config.resizable ? GLFW_TRUE : GLFW_FALSE);

    // Create window
    m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);
    if (!m_window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    // Make context current
    glfwMakeContextCurrent(m_window);

    // Enable vsync
    glfwSwapInterval(config.vsync ? 1 : 0);

    // Set user pointer to this instance
    glfwSetWindowUserPointer(m_window, this);

    // Setup callbacks
    setupCallbacks();

    // Store windowed position
    glfwGetWindowPos(m_window, &m_windowedPosX, &m_windowedPosY);

    return true;
}

void Window::destroy() const
{
    if (m_window) {
        glfwDestroyWindow(m_window);
    }
    // Ne pas appeler glfwTerminate() ici car d'autres instances pourraient l'utiliser
}

void Window::shutdown() {
    if (m_window) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
    // Appeler glfwTerminate() seulement lors de la destruction finale
    static bool terminated = false;
    if (!terminated) {
        glfwTerminate();
        terminated = true;
    }
}

bool Window::shouldClose() const {
    return m_window ? glfwWindowShouldClose(m_window) : true;
}

void Window::setShouldClose(bool close) {
    if (m_window) {
        glfwSetWindowShouldClose(m_window, close ? GLFW_TRUE : GLFW_FALSE);
    }
}

void Window::swapBuffers() {
    if (m_window) {
        glfwSwapBuffers(m_window);
    }
}

void Window::pollEvents() {
    glfwPollEvents();
}

void Window::toggleFullscreen() {
    if (!m_window) return;

    if (m_isFullscreen) {
        // Switch to windowed mode
        glfwSetWindowMonitor(m_window, nullptr, m_windowedPosX, m_windowedPosY, 
                           m_windowedWidth, m_windowedHeight, 0);
        m_isFullscreen = false;
        m_width = m_windowedWidth;
        m_height = m_windowedHeight;
    } else {
        // Store current windowed position and size
        glfwGetWindowPos(m_window, &m_windowedPosX, &m_windowedPosY);
        glfwGetWindowSize(m_window, &m_windowedWidth, &m_windowedHeight);

        // Switch to fullscreen
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        glfwSetWindowMonitor(m_window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
        m_isFullscreen = true;
        m_width = mode->width;
        m_height = mode->height;
    }
}

bool Window::isFullscreen() const {
    return m_isFullscreen;
}

void Window::getFramebufferSize(int& width, int& height) const {
    if (m_window) {
        glfwGetFramebufferSize(m_window, &width, &height);
    }
}

void Window::setCursorMode(int mode) {
    if (m_window) {
        glfwSetInputMode(m_window, GLFW_CURSOR, mode);
    }
}

void Window::getCursorPos(double& xpos, double& ypos) const {
    if (m_window) {
        glfwGetCursorPos(m_window, &xpos, &ypos);
    }
}

void Window::setKeyCallback(const KeyCallback& callback) {
    m_keyCallback = callback;
}

void Window::setMouseButtonCallback(const MouseButtonCallback& callback) {
    m_mouseButtonCallback = callback;
}

void Window::setCursorPosCallback(const CursorPosCallback& callback) {
    m_cursorPosCallback = callback;
}

void Window::setFramebufferSizeCallback(const FramebufferSizeCallback& callback) {
    m_framebufferSizeCallback = callback;
}

void Window::setWindowSizeCallback(const WindowSizeCallback& callback) {
    m_windowSizeCallback = callback;
}

void Window::setUserPointer(void* pointer) {
    if (m_window) {
        glfwSetWindowUserPointer(m_window, pointer);
    }
}

void* Window::getUserPointer() const {
    return m_window ? glfwGetWindowUserPointer(m_window) : nullptr;
}

bool Window::initializeGLFW() {
    glfwSetErrorCallback(glfwErrorCallback);

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    return true;
}

void Window::setupCallbacks() {
    if (!m_window) return;

    glfwSetKeyCallback(m_window, glfwKeyCallback);
    glfwSetMouseButtonCallback(m_window, glfwMouseButtonCallback);
    glfwSetCursorPosCallback(m_window, glfwCursorPosCallback);
    glfwSetFramebufferSizeCallback(m_window, glfwFramebufferSizeCallback);
    glfwSetWindowSizeCallback(m_window, glfwWindowSizeCallback);
}

// Static GLFW callbacks
void Window::glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (win && win->m_keyCallback) {
        win->m_keyCallback(key, scancode, action, mods);
    }
}

void Window::glfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (win && win->m_mouseButtonCallback) {
        win->m_mouseButtonCallback(button, action, mods);
    }
}

void Window::glfwCursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (win && win->m_cursorPosCallback) {
        win->m_cursorPosCallback(xpos, ypos);
    }
}

void Window::glfwFramebufferSizeCallback(GLFWwindow* window, int width, int height) {
    Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (win) {
        if (win->m_framebufferSizeCallback) {
            win->m_framebufferSizeCallback(width, height);
        }
    }
}

void Window::glfwWindowSizeCallback(GLFWwindow* window, int width, int height) {
    Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (win) {
        win->m_width = width;
        win->m_height = height;
        if (win->m_windowSizeCallback) {
            win->m_windowSizeCallback(width, height);
        }
    }
}

void Window::glfwErrorCallback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}