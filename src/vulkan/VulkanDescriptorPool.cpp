#include "VulkanDescriptorPool.h"
#include "VulkanDevice.h"
#include "core/Logger.h"
#include <stdexcept>

VulkanDescriptorPool::VulkanDescriptorPool(VulkanDevice* device) 
    : device(device), descriptorPool(VK_NULL_HANDLE) {
    Logger::info("Creating Vulkan Descriptor Pool");
    createDescriptorPool();
    Logger::info("Vulkan Descriptor Pool created successfully");
}

VulkanDescriptorPool::~VulkanDescriptorPool() {
    Logger::info("Destroying Vulkan Descriptor Pool");
    if (descriptorPool) {
        vkDestroyDescriptorPool(device->getDevice(), descriptorPool, nullptr);
    }
}

void VulkanDescriptorPool::createDescriptorPool() {
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = static_cast<uint32_t>(1000); // Adjust as needed
    
    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = static_cast<uint32_t>(1000); // Adjust as needed
    
    if (vkCreateDescriptorPool(device->getDevice(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor pool!");
    }
} 