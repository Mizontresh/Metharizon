#include "Window.h"
#include "core/Logger.h"
#include <stdexcept>

Window::Window(const std::string& title, int width, int height) 
    : title(title), width(width), height(height), window(nullptr) {
    
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }
    
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    
    window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }
    
    // Set up callbacks
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetKeyCallback(window, keyCallbackStatic);
    glfwSetMouseButtonCallback(window, mouseButtonCallbackStatic);
    glfwSetCursorPosCallback(window, mouseMoveCallbackStatic);
    
    Logger::info("Window created: " + title + " (" + std::to_string(width) + "x" + std::to_string(height) + ")");
}

Window::~Window() {
    if (window) {
        glfwDestroyWindow(window);
    }
    Logger::info("Window destroyed");
}

bool Window::shouldClose() const {
    return glfwWindowShouldClose(window);
}

void Window::pollEvents() {
    glfwPollEvents();
}

VkSurfaceKHR Window::createSurface(VkInstance instance) {
    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface");
    }
    return surface;
}

void Window::setResizeCallback(std::function<void(int, int)> callback) {
    resizeCallback = callback;
}

void Window::setKeyCallback(std::function<void(int, int, int)> callback) {
    keyCallback = callback;
}

void Window::setMouseButtonCallback(std::function<void(int, int, int)> callback) {
    mouseButtonCallback = callback;
}

void Window::setMouseMoveCallback(std::function<void(double, double)> callback) {
    mouseMoveCallback = callback;
}

bool Window::isKeyPressed(int key) const {
    return glfwGetKey(window, key) == GLFW_PRESS;
}

bool Window::isMouseButtonPressed(int button) const {
    return glfwGetMouseButton(window, button) == GLFW_PRESS;
}

void Window::getMousePosition(double& x, double& y) const {
    glfwGetCursorPos(window, &x, &y);
}

// Static callback functions
void Window::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (win && win->resizeCallback) {
        win->resizeCallback(width, height);
    }
}

void Window::keyCallbackStatic(GLFWwindow* window, int key, int scancode, int action, int mods) {
    Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (win && win->keyCallback) {
        win->keyCallback(key, scancode, action);
    }
}

void Window::mouseButtonCallbackStatic(GLFWwindow* window, int button, int action, int mods) {
    Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (win && win->mouseButtonCallback) {
        win->mouseButtonCallback(button, action, mods);
    }
}

void Window::mouseMoveCallbackStatic(GLFWwindow* window, double xpos, double ypos) {
    Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (win && win->mouseMoveCallback) {
        win->mouseMoveCallback(xpos, ypos);
    }
} 