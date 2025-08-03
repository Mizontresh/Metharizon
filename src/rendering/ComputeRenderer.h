#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <glm/glm.hpp>

// Forward declarations
class VulkanDevice;
class VulkanCommandPool;
class VulkanSwapChain;
class VulkanComputePipeline;

class ComputeRenderer {
public:
    ComputeRenderer(VulkanDevice* device, VulkanCommandPool* commandPool);
    ~ComputeRenderer();

    void createCommandBuffers();
    void createDescriptorSet();
    void createOutputImage(VulkanSwapChain* swapChain);
    
    void updateCameraData(const glm::vec3& cameraPos, float time, const glm::mat4& viewMatrix, float aspectRatio);
    void recordCommandBuffer(uint32_t imageIndex, size_t currentFrame, VkImage swapChainImage);
    
    VkCommandBuffer getCommandBuffer(size_t frameIndex);
    
    void cleanup();

private:
    VulkanDevice* device;
    VulkanCommandPool* commandPool;
    VulkanComputePipeline* computePipeline;
    
    // Descriptor set for compute shader
    VkDescriptorSet descriptorSet;
    VkDescriptorPool descriptorPool;
    
    // Command buffers
    std::vector<VkCommandBuffer> commandBuffers;
    
    // Camera data
    glm::vec3 cameraPos;
    float time;
    glm::mat4 viewMatrix;
    float aspectRatio;
    
    // Extent for output image
    VkExtent2D imageExtent;
    
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    void copyImageToSwapChain(VkCommandBuffer commandBuffer, VkImage srcImage, VkImage dstImage, VkExtent2D extent);
}; 