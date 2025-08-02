#pragma once
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <string>
#include <functional>

class Window {
public:
    Window(const std::string& title, int width, int height);
    ~Window();
    
    // Disable copy constructor and assignment
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    
    // Window management
    bool shouldClose() const;
    void pollEvents();
    GLFWwindow* getHandle() const { return window; }
    
    // Surface creation
    VkSurfaceKHR createSurface(VkInstance instance);
    
    // Callbacks
    void setResizeCallback(std::function<void(int, int)> callback);
    void setKeyCallback(std::function<void(int, int, int)> callback);
    void setMouseButtonCallback(std::function<void(int, int, int)> callback);
    void setMouseMoveCallback(std::function<void(double, double)> callback);
    
    // Input state
    bool isKeyPressed(int key) const;
    bool isMouseButtonPressed(int button) const;
    void getMousePosition(double& x, double& y) const;
    
private:
    GLFWwindow* window;
    std::string title;
    int width, height;
    
    // Callback functions
    std::function<void(int, int)> resizeCallback;
    std::function<void(int, int, int)> keyCallback;
    std::function<void(int, int, int)> mouseButtonCallback;
    std::function<void(double, double)> mouseMoveCallback;
    
    // Static callback functions for GLFW
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void keyCallbackStatic(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouseButtonCallbackStatic(GLFWwindow* window, int button, int action, int mods);
    static void mouseMoveCallbackStatic(GLFWwindow* window, double xpos, double ypos);
}; 