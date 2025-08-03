#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <string>

class VulkanDevice;

class VulkanComputePipeline {
public:
    VulkanComputePipeline(VulkanDevice* device);
    ~VulkanComputePipeline();
    
    // Disable copy constructor and assignment
    VulkanComputePipeline(const VulkanComputePipeline&) = delete;
    VulkanComputePipeline& operator=(const VulkanComputePipeline&) = delete;
    
    // Getters
    VkPipeline getPipeline() const { return computePipeline; }
    VkPipelineLayout getPipelineLayout() const { return pipelineLayout; }
    VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }
    
    // Create output image for compute shader
    void createOutputImage(VkExtent2D extent);
    VkImage getOutputImage() const { return outputImage; }
    VkImageView getOutputImageView() const { return outputImageView; }
    
private:
    VulkanDevice* device;
    VkPipeline computePipeline;
    VkPipelineLayout pipelineLayout;
    VkDescriptorSetLayout descriptorSetLayout;
    
    // Output image for compute shader
    VkImage outputImage;
    VkDeviceMemory outputImageMemory;
    VkImageView outputImageView;
    
    void createComputePipeline();
    void createDescriptorSetLayout();
    void createOutputImageResources(VkExtent2D extent);
    std::vector<char> readFile(const std::string& filename);
    VkShaderModule createShaderModule(const std::vector<char>& code);
}; 