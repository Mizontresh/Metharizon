#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class VulkanDevice;
class VulkanPipeline;
class VulkanCommandPool;
class VulkanSwapChain;
class VulkanRenderPass;

class RaymarchRenderer {
public:
    RaymarchRenderer(VulkanDevice* device, VulkanPipeline* pipeline, VulkanCommandPool* commandPool);
    ~RaymarchRenderer();
    
    // Disable copy constructor and assignment
    RaymarchRenderer(const RaymarchRenderer&) = delete;
    RaymarchRenderer& operator=(const RaymarchRenderer&) = delete;
    
    // Rendering
    void recordCommandBuffer(uint32_t imageIndex, size_t currentFrame);
    VkCommandBuffer getCommandBuffer(size_t frameIndex);
    
    // Setup
    void createFramebuffers(VulkanSwapChain* swapChain, VulkanRenderPass* renderPass);
    void createCommandBuffers();
    void createUniformBuffer();
    void createDescriptorSet();
    void cleanupFramebuffers();
    
    // Update
    void updateUniformBuffer(const glm::mat4& view, const glm::mat4& proj, const glm::vec3& cameraPos, float time);
    void updateCameraData(const glm::vec3& cameraPos, float time, const glm::mat4& viewMatrix, float aspectRatio);
    
    // Cleanup
    void cleanup();
    
private:
    VulkanDevice* device;
    VulkanPipeline* pipeline;
    VulkanCommandPool* commandPool;
    VulkanRenderPass* renderPass;
    VulkanSwapChain* swapChain;
    
    std::vector<VkFramebuffer> swapChainFramebuffers;
    std::vector<VkCommandBuffer> commandBuffers;
    
    // Uniform buffer and descriptor set
    VkBuffer uniformBuffer;
    VkDeviceMemory uniformBufferMemory;
    VkDescriptorSet descriptorSet;
    
    // Camera data for push constants
    glm::vec3 cameraPos;
    float time;
    glm::mat4 viewMatrix;
    float aspectRatio;
    
    void createFramebuffer(VkImageView imageView, VkFramebuffer& framebuffer, VkExtent2D extent);
}; 