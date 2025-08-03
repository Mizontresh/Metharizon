#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <string>

class VulkanDevice;

class VulkanPhysicsPipeline {
public:
    explicit VulkanPhysicsPipeline(VulkanDevice* device);
    ~VulkanPhysicsPipeline();

    VkPipeline              getPipeline()        const { return computePipeline; }
    VkPipelineLayout        getPipelineLayout()  const { return pipelineLayout; }
    VkDescriptorSetLayout   getDescriptorSetLayout() const { return descriptorSetLayout; }

private:
    VulkanDevice* device;

    VkPipeline              computePipeline   = VK_NULL_HANDLE;
    VkPipelineLayout        pipelineLayout    = VK_NULL_HANDLE;
    VkDescriptorSetLayout   descriptorSetLayout = VK_NULL_HANDLE;

    // helpers
    std::vector<char> readFile(const std::string& filename);
    VkShaderModule createShaderModule(const std::vector<char>& code);

    void createDescriptorSetLayout();
    void createComputePipeline();
};
