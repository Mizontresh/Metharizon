#pragma once
#include <memory>
#include <vector>
#include <vulkan/vulkan.h>
#include "core/Window.h"
#include "core/Camera.h"

// Forward declarations
class VulkanInstance;
class VulkanDevice;
class VulkanSwapChain;
class VulkanRenderPass;
class VulkanPipeline;
class VulkanCommandPool;
class VulkanDescriptorPool;
class RaymarchRenderer;
class ComputeRenderer;


class VulkanApplication {
public:
    VulkanApplication(Window* window);
    ~VulkanApplication();
    
    // Disable copy constructor and assignment
    VulkanApplication(const VulkanApplication&) = delete;
    VulkanApplication& operator=(const VulkanApplication&) = delete;
    
    // Main loop
    void render();
    
    // Input handling
    void handleInput();
    
    // Camera
    std::unique_ptr<Camera> camera;
    
    // Rendering components
    std::unique_ptr<ComputeRenderer> computeRenderer;
    
private:
    Window* window;
    
    // Vulkan components
    std::unique_ptr<VulkanInstance> instance;
    std::unique_ptr<VulkanDevice> device;
    std::unique_ptr<VulkanSwapChain> swapChain;
    std::unique_ptr<VulkanCommandPool> commandPool;
    std::unique_ptr<VulkanDescriptorPool> descriptorPool;
    
    // Synchronization
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    size_t currentFrame = 0;
    
    static const int MAX_FRAMES_IN_FLIGHT = 2;
    
    // Callbacks
    void onWindowResize(int width, int height);
    void onKeyPress(int key, int scancode, int action);
    void onMouseButton(int button, int action, int mods);
    void onMouseMove(double xpos, double ypos);
    
    // Helper functions
    void createSyncObjects();
    void cleanupSwapChain();
    void recreateSwapChain();
}; 