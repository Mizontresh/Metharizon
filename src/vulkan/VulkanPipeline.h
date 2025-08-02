#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <string>

class VulkanDevice;
class VulkanRenderPass;

class VulkanPipeline {
public:
    VulkanPipeline(VulkanDevice* device, VulkanRenderPass* renderPass);
    ~VulkanPipeline();
    
    // Disable copy constructor and assignment
    VulkanPipeline(const VulkanPipeline&) = delete;
    VulkanPipeline& operator=(const VulkanPipeline&) = delete;
    
    // Getters
    VkPipeline getPipeline() const { return graphicsPipeline; }
    VkPipelineLayout getPipelineLayout() const { return pipelineLayout; }
    VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }
    
private:
    VulkanDevice* device;
    VulkanRenderPass* renderPass;
    VkPipeline graphicsPipeline;
    VkPipelineLayout pipelineLayout;
    VkDescriptorSetLayout descriptorSetLayout;
    
    void createGraphicsPipeline();
    std::vector<char> readFile(const std::string& filename);
    VkShaderModule createShaderModule(const std::vector<char>& code);
}; 