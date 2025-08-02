#pragma once
#include <vulkan/vulkan.h>

class VulkanDevice;
class VulkanSwapChain;

class VulkanRenderPass {
public:
    VulkanRenderPass(VulkanDevice* device, VulkanSwapChain* swapChain);
    ~VulkanRenderPass();
    
    // Disable copy constructor and assignment
    VulkanRenderPass(const VulkanRenderPass&) = delete;
    VulkanRenderPass& operator=(const VulkanRenderPass&) = delete;
    
    // Getters
    VkRenderPass getRenderPass() const { return renderPass; }
    
private:
    VulkanDevice* device;
    VulkanSwapChain* swapChain;
    VkRenderPass renderPass;
    
    void createRenderPass();
}; 