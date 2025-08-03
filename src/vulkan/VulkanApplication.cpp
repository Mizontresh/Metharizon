#include "VulkanApplication.h"
#include "core/Logger.h"
#include "core/Camera.h"
#include "vulkan/VulkanInstance.h"
#include "vulkan/VulkanDevice.h"
#include "vulkan/VulkanSwapChain.h"

#include "vulkan/VulkanCommandPool.h"
#include "vulkan/VulkanDescriptorPool.h"

#include "rendering/ComputeRenderer.h"
#include "rendering/PhysicsComputeRenderer.h"

#include <stdexcept>
#include <GLFW/glfw3.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

VulkanApplication::VulkanApplication(Window* window) : window(window) {
    Logger::info("Initializing Vulkan Application");
    
    // Initialize camera
    camera = std::make_unique<Camera>();
    
    // Hide mouse cursor for infinite rotation
    glfwSetInputMode(window->getHandle(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    // Initialize Vulkan components
    instance = std::make_unique<VulkanInstance>();
    device = std::make_unique<VulkanDevice>(instance.get(), window);
    swapChain = std::make_unique<VulkanSwapChain>(device.get(), window);
    commandPool = std::make_unique<VulkanCommandPool>(device.get());
    descriptorPool = std::make_unique<VulkanDescriptorPool>(device.get());
    
    // Initialize GPU physics renderer
    physicsRenderer = std::make_unique<PhysicsComputeRenderer>(device.get(), commandPool.get());

    // Initialize compute renderer
    computeRenderer = std::make_unique<ComputeRenderer>(device.get(), commandPool.get());
    computeRenderer->createOutputImage(swapChain.get());
    computeRenderer->createDescriptorSet(physicsRenderer->getStateBuffer());
    

    
    // Create synchronization objects
    createSyncObjects();
    
    // Set up input callbacks
    window->setResizeCallback([this](int width, int height) {
        onWindowResize(width, height);
    });
    
    window->setKeyCallback([this](int key, int scancode, int action) {
        onKeyPress(key, scancode, action);
    });
    
    window->setMouseButtonCallback([this](int button, int action, int mods) {
        onMouseButton(button, action, mods);
    });
    
    window->setMouseMoveCallback([this](double xpos, double ypos) {
        onMouseMove(xpos, ypos);
    });
    
    Logger::info("Vulkan Application initialized successfully");
}

VulkanApplication::~VulkanApplication() {
    Logger::info("Cleaning up Vulkan Application");

    vkDeviceWaitIdle(device->getDevice());

    // Destroy GPU and CPU renderers before tearing down device
    physicsRenderer.reset();
    computeRenderer.reset();
    
    // Cleanup synchronization objects
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(device->getDevice(), renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(device->getDevice(), imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(device->getDevice(), inFlightFences[i], nullptr);
    }
    
    Logger::info("Vulkan Application cleanup completed");
}

void VulkanApplication::render() {
    // Wait for the previous frame to finish
    vkWaitForFences(device->getDevice(), 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
    
    // Acquire the next image from the swap chain
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(device->getDevice(), swapChain->getSwapChain(), UINT64_MAX, 
                                           imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapChain();
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("Failed to acquire swap chain image");
    }
    
    // Only reset the fence if we are submitting work
    vkResetFences(device->getDevice(), 1, &inFlightFences[currentFrame]);
    
    // Get current window size for proper aspect ratio
    int width, height;
    glfwGetFramebufferSize(window->getHandle(), &width, &height);
    float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
    glm::vec2 screenSize(static_cast<float>(width), static_cast<float>(height));
    
    // Debug logging for aspect ratio
    static float lastAspectRatio = 0.0f;
    static int lastWidth = 0, lastHeight = 0;
    if (std::abs(aspectRatio - lastAspectRatio) > 0.01f || width != lastWidth || height != lastHeight) {
        Logger::info("Window/Resolution changed: " + std::to_string(lastWidth) + "x" + std::to_string(lastHeight) + 
                    " -> " + std::to_string(width) + "x" + std::to_string(height) + 
                    " (aspect: " + std::to_string(lastAspectRatio) + " -> " + std::to_string(aspectRatio) + ")");
        lastAspectRatio = aspectRatio;
        lastWidth = width;
        lastHeight = height;
    }
    
    // Handle input and update camera
    handleInput();
    
    // Physics step (real dt)
    static double lastPhysicsTime = glfwGetTime();
    double curr = glfwGetTime();
    float dtPhysics = static_cast<float>(curr - lastPhysicsTime);
    lastPhysicsTime = curr;
    /* CPU physics disabled – GPU authoritative */

    // GPU physics step
    if (physicsRenderer) {
        physicsRenderer->updatePushConstants(dtPhysics, 1.0f /*G*/, 1.0f /*restitution*/);
        physicsRenderer->dispatch(device->getGraphicsQueue());
    }

    // Update camera data for shader
    glm::mat4 view = camera->getViewMatrix();
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
    proj[1][1] *= -1; // Flip Y for Vulkan
    
    static float timeAccum = 0.0f;
    timeAccum += dtPhysics;
    
    // Safety check: ensure renderer is valid before using it
    if (!computeRenderer) {
        Logger::error("Compute renderer is null, skipping frame");
        return;
    }
    
    computeRenderer->updateCameraData(camera->getPosition(), timeAccum, view, aspectRatio);

    
    // Debug: Log camera data being sent to shader
    static int debugFrame = 0;
    debugFrame++;
    if (debugFrame % 60 == 0) {
        Logger::info("Camera pos: (" + std::to_string(camera->getPosition().x) + ", " + std::to_string(camera->getPosition().y) + ", " + std::to_string(camera->getPosition().z) + ")");
    }
    
    // Record command buffer
    VkImage swapChainImage = swapChain->getSwapChainImages()[imageIndex];
    computeRenderer->recordCommandBuffer(imageIndex, currentFrame, swapChainImage);
    
    // Submit command buffer
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    
    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    
    VkCommandBuffer commandBuffer = computeRenderer->getCommandBuffer(currentFrame);
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    
    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;
    
    if (vkQueueSubmit(device->getGraphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("Failed to submit draw command buffer");
    }
    
    // Present the frame
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    
    VkSwapchainKHR swapChains[] = {swapChain->getSwapChain()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    
    result = vkQueuePresentKHR(device->getPresentQueue(), &presentInfo);
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        recreateSwapChain();
    } else if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to present swap chain image");
    }
    
    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VulkanApplication::onWindowResize(int width, int height) {
    Logger::info("Window resized to " + std::to_string(width) + "x" + std::to_string(height));
    Logger::info("Triggering swap chain recreation...");
    recreateSwapChain();
}

void VulkanApplication::createSyncObjects() {
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(device->getDevice(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(device->getDevice(), &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(device->getDevice(), &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create synchronization objects for a frame");
        }
    }
    
    Logger::info("Created " + std::to_string(MAX_FRAMES_IN_FLIGHT) + " synchronization objects");
}

void VulkanApplication::cleanupSwapChain() {
    Logger::info("Cleaning up swap chain resources...");
    
    // Clean up the renderers first (they depend on swap chain resources)
    if (computeRenderer) {
        computeRenderer->cleanup();
        computeRenderer.reset();
    }
    
    // Clean up swap chain last
    if (swapChain) {
        swapChain.reset();
    }
    
    Logger::info("Swap chain cleanup completed");
}

void VulkanApplication::recreateSwapChain() {
    Logger::info("Starting swap chain recreation...");
    
    int width = 0, height = 0;
    glfwGetFramebufferSize(window->getHandle(), &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(window->getHandle(), &width, &height);
        glfwWaitEvents();
    }
    
    Logger::info("New window size: " + std::to_string(width) + "x" + std::to_string(height));
    
    vkDeviceWaitIdle(device->getDevice());
    
    Logger::info("Cleaning up old swap chain...");
    cleanupSwapChain();
    
    Logger::info("Creating new swap chain...");
    // Recreate swap chain and related objects
    swapChain = std::make_unique<VulkanSwapChain>(device.get(), window);
    
    Logger::info("Creating new renderers...");
    // Create a completely new renderer instances
    try {
        computeRenderer = std::make_unique<ComputeRenderer>(device.get(), commandPool.get());
        computeRenderer->createOutputImage(swapChain.get());
        computeRenderer->createDescriptorSet(physicsRenderer->getStateBuffer());
        Logger::info("New compute renderer created successfully");
    } catch (const std::exception& e) {
        Logger::error("Failed to create new renderer: " + std::string(e.what()));
        throw;
    }
    
    Logger::info("Swap chain recreated successfully");
} 

void VulkanApplication::handleInput() {
    // Update camera based on current input state
    camera->moveForwardPressed = window->isKeyPressed(GLFW_KEY_W);
    camera->moveBackwardPressed = window->isKeyPressed(GLFW_KEY_S);
    camera->moveLeftPressed = window->isKeyPressed(GLFW_KEY_A);
    camera->moveRightPressed = window->isKeyPressed(GLFW_KEY_D);
    camera->moveUpPressed = window->isKeyPressed(GLFW_KEY_LEFT_SHIFT);
    camera->moveDownPressed = window->isKeyPressed(GLFW_KEY_SPACE);
    
    // Update camera
    static float lastTime = 0.0f;
    float currentTime = static_cast<float>(glfwGetTime());
    float deltaTime = currentTime - lastTime;
    lastTime = currentTime;
    
    camera->update(deltaTime);
}

void VulkanApplication::onKeyPress(int key, int scancode, int action) {
    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(window->getHandle(), GLFW_TRUE);
                break;
            case GLFW_KEY_F11:
                Logger::info("F11 pressed - toggling fullscreen");
                
                // Get current window state
                GLFWmonitor* currentMonitor = glfwGetWindowMonitor(window->getHandle());
                bool isCurrentlyFullscreen = (currentMonitor != nullptr);
                
                if (!isCurrentlyFullscreen) {
                    Logger::info("Switching to fullscreen mode");
                    // Get the primary monitor
                    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
                    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
                    
                    // Store current window position and size for restoration
                    int xpos, ypos, width, height;
                    glfwGetWindowPos(window->getHandle(), &xpos, &ypos);
                    glfwGetWindowSize(window->getHandle(), &width, &height);
                    
                    // Set fullscreen
                    glfwSetWindowMonitor(window->getHandle(), monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
                } else {
                    Logger::info("Switching to windowed mode");
                    // Return to windowed mode with a reasonable size
                    glfwSetWindowMonitor(window->getHandle(), nullptr, 100, 100, 1280, 720, 0);
                }
                
                // Wait a bit for the window to settle before processing events
                glfwWaitEventsTimeout(0.1);
                break;
        }
    }
}

void VulkanApplication::onMouseButton(int button, int action, int mods) {
    // Mouse movement is now automatic - no need to hold any button
    // This function is kept for potential future use
}

void VulkanApplication::onMouseMove(double xpos, double ypos) {
    // Mouse movement for camera rotation
    static bool firstMouse = true;
    static double lastX = 0.0;
    static double lastY = 0.0;
    
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
        return;
    }
    
    // Calculate delta from last position
    double deltaX = xpos - lastX;
    double deltaY = ypos - lastY;
    
    // Rotate camera based on mouse movement (fixed Y-axis)
    camera->rotateYaw(static_cast<float>(deltaX * camera->mouseSensitivity));
    camera->rotatePitch(static_cast<float>(-deltaY * camera->mouseSensitivity)); // Inverted Y-axis for inverted mouse
    
    // Update last position
    lastX = xpos;
    lastY = ypos;
} 