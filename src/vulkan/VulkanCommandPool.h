#pragma once
#include <vulkan/vulkan.h>
#include <vector>

class VulkanDevice;

class VulkanCommandPool {
public:
    VulkanCommandPool(VulkanDevice* device);
    ~VulkanCommandPool();
    
    // Disable copy constructor and assignment
    VulkanCommandPool(const VulkanCommandPool&) = delete;
    VulkanCommandPool& operator=(const VulkanCommandPool&) = delete;
    
    // Command buffer management
    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);
    
    // Getters
    VkCommandPool getCommandPool() const { return commandPool; }
    
private:
    VulkanDevice* device;
    VkCommandPool commandPool;
    
    void createCommandPool();
}; 