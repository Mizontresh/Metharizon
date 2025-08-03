#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "core/Window.h"

class VulkanDevice;

class VulkanSwapChain {
public:
    VulkanSwapChain(VulkanDevice* device, Window* window);
    ~VulkanSwapChain();
    
    // Disable copy constructor and assignment
    VulkanSwapChain(const VulkanSwapChain&) = delete;
    VulkanSwapChain& operator=(const VulkanSwapChain&) = delete;
    
    // Getters
    VkSwapchainKHR getSwapChain() const { return swapChain; }
    VkFormat getSwapChainImageFormat() const { return swapChainImageFormat; }
    VkExtent2D getSwapChainExtent() const { return swapChainExtent; }
    const std::vector<VkImageView>& getSwapChainImageViews() const { return swapChainImageViews; }
    const std::vector<VkImage>& getSwapChainImages() const { return swapChainImages; }
    size_t getImageCount() const { return swapChainImages.size(); }
    
    // Recreation
    void recreate();
    
private:
    VulkanDevice* device;
    Window* window;
    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    
    // Initialization methods
    void createSwapChain();
    void createImageViews();
    
    // Helper methods
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    
    // Cleanup
    void cleanup();
}; 