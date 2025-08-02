#pragma once
#include <vulkan/vulkan.h>
#include <vector>

class VulkanDevice;

class VulkanDescriptorPool {
public:
    VulkanDescriptorPool(VulkanDevice* device);
    ~VulkanDescriptorPool();
    
    // Disable copy constructor and assignment
    VulkanDescriptorPool(const VulkanDescriptorPool&) = delete;
    VulkanDescriptorPool& operator=(const VulkanDescriptorPool&) = delete;
    
    // Getters
    VkDescriptorPool getDescriptorPool() const { return descriptorPool; }
    
private:
    VulkanDevice* device;
    VkDescriptorPool descriptorPool;
    
    void createDescriptorPool();
}; 