#include "VulkanPhysicsPipeline.h"
#include "VulkanDevice.h"
#include "core/Logger.h"
#include <stdexcept>
#include <fstream>

VulkanPhysicsPipeline::VulkanPhysicsPipeline(VulkanDevice* device) : device(device) {
    Logger::info("Creating Vulkan Physics Compute Pipeline");
    createDescriptorSetLayout();
    createComputePipeline();
}

VulkanPhysicsPipeline::~VulkanPhysicsPipeline() {
    Logger::info("Destroying Vulkan Physics Compute Pipeline");

    if (computePipeline != VK_NULL_HANDLE)
        vkDestroyPipeline(device->getDevice(), computePipeline, nullptr);
    if (pipelineLayout != VK_NULL_HANDLE)
        vkDestroyPipelineLayout(device->getDevice(), pipelineLayout, nullptr);
    if (descriptorSetLayout != VK_NULL_HANDLE)
        vkDestroyDescriptorSetLayout(device->getDevice(), descriptorSetLayout, nullptr);
}

void VulkanPhysicsPipeline::createDescriptorSetLayout() {
    // One storage buffer binding (binding = 0)
    VkDescriptorSetLayoutBinding stateBinding{};
    stateBinding.binding = 0;
    stateBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    stateBinding.descriptorCount = 1;
    stateBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    stateBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &stateBinding;

    if (vkCreateDescriptorSetLayout(device->getDevice(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create physics descriptor set layout");
    }
}

void VulkanPhysicsPipeline::createComputePipeline() {
    // Load shader module
    auto code = readFile("shaders/physics_step.comp.spv");
    VkShaderModule shaderModule = createShaderModule(code);

    VkPipelineShaderStageCreateInfo stageInfo{};
    stageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stageInfo.stage  = VK_SHADER_STAGE_COMPUTE_BIT;
    stageInfo.module = shaderModule;
    stageInfo.pName  = "main";

    VkPushConstantRange pcRange{};
    pcRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    pcRange.offset = 0;
    pcRange.size   = sizeof(float) * 3; // dt, G, restitution

    VkPipelineLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutInfo.setLayoutCount = 1;
    layoutInfo.pSetLayouts    = &descriptorSetLayout;
    layoutInfo.pushConstantRangeCount = 1;
    layoutInfo.pPushConstantRanges    = &pcRange;

    if (vkCreatePipelineLayout(device->getDevice(), &layoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create physics pipeline layout");
    }

    VkComputePipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.stage = stageInfo;
    pipelineInfo.layout = pipelineLayout;

    if (vkCreateComputePipelines(device->getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &computePipeline) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create physics compute pipeline");
    }

    vkDestroyShaderModule(device->getDevice(), shaderModule, nullptr);
}

std::vector<char> VulkanPhysicsPipeline::readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open shader file: " + filename);
    }
    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();
    return buffer;
}

VkShaderModule VulkanPhysicsPipeline::createShaderModule(const std::vector<char>& code) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode    = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device->getDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create shader module");
    }
    return shaderModule;
}
