#include "VulkanCommandPool.h"
#include "VulkanDevice.h"
#include "core/Logger.h"
#include <stdexcept>

VulkanCommandPool::VulkanCommandPool(VulkanDevice* device) 
    : device(device), commandPool(VK_NULL_HANDLE) {
    Logger::info("Creating Vulkan Command Pool");
    createCommandPool();
    Logger::info("Vulkan Command Pool created successfully");
}

VulkanCommandPool::~VulkanCommandPool() {
    Logger::info("Destroying Vulkan Command Pool");
    if (commandPool) {
        vkDestroyCommandPool(device->getDevice(), commandPool, nullptr);
    }
}

void VulkanCommandPool::createCommandPool() {
    auto queueFamilyIndices = device->findQueueFamilies();
    
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    
    if (vkCreateCommandPool(device->getDevice(), &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create command pool!");
    }
}

VkCommandBuffer VulkanCommandPool::beginSingleTimeCommands() {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;
    
    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device->getDevice(), &allocInfo, &commandBuffer);
    
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    
    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    
    return commandBuffer;
}

void VulkanCommandPool::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);
    
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    
    vkQueueSubmit(device->getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(device->getGraphicsQueue());
    
    vkFreeCommandBuffers(device->getDevice(), commandPool, 1, &commandBuffer);
} 