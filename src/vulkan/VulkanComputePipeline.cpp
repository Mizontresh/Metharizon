#include "VulkanComputePipeline.h"
#include "VulkanDevice.h"
#include "core/Logger.h"
#include <stdexcept>
#include <fstream>

VulkanComputePipeline::VulkanComputePipeline(VulkanDevice* device) : device(device) {
    Logger::info("Creating Vulkan Compute Pipeline");
    createDescriptorSetLayout();
    createComputePipeline();
}

VulkanComputePipeline::~VulkanComputePipeline() {
    Logger::info("Destroying Vulkan Compute Pipeline");
    
    if (outputImageView != VK_NULL_HANDLE) {
        vkDestroyImageView(device->getDevice(), outputImageView, nullptr);
    }
    
    if (outputImage != VK_NULL_HANDLE) {
        vkDestroyImage(device->getDevice(), outputImage, nullptr);
    }
    
    if (outputImageMemory != VK_NULL_HANDLE) {
        vkFreeMemory(device->getDevice(), outputImageMemory, nullptr);
    }
    
    if (computePipeline != VK_NULL_HANDLE) {
        vkDestroyPipeline(device->getDevice(), computePipeline, nullptr);
    }
    
    if (pipelineLayout != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(device->getDevice(), pipelineLayout, nullptr);
    }
    
    if (descriptorSetLayout != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(device->getDevice(), descriptorSetLayout, nullptr);
    }
}

void VulkanComputePipeline::createDescriptorSetLayout() {
    Logger::info("Creating compute descriptor set layout");
    
    // Only storage image binding - no uniform buffer binding needed for push constants
    VkDescriptorSetLayoutBinding imageBinding{};
    imageBinding.binding = 1;
    imageBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    imageBinding.descriptorCount = 1;
    imageBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &imageBinding;
    
    if (vkCreateDescriptorSetLayout(device->getDevice(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create compute descriptor set layout!");
    }
    
    Logger::info("Compute descriptor set layout created successfully");
}

void VulkanComputePipeline::createComputePipeline() {
    Logger::info("Creating compute pipeline");
    
    // Load compute shader
    auto computeShaderCode = readFile("Release/shaders/raymarch.comp.spv");
    VkShaderModule computeShaderModule = createShaderModule(computeShaderCode);
    
    VkPipelineShaderStageCreateInfo computeShaderStageInfo{};
    computeShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    computeShaderStageInfo.module = computeShaderModule;
    computeShaderStageInfo.pName = "main";
    
    // Create pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
    
    // Push constant range
    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(float) * 21; // vec3 + float + mat4 + float (12+4+64+4 = 84 bytes = 21 floats)
    
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    
    if (vkCreatePipelineLayout(device->getDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create compute pipeline layout!");
    }
    
    // Create compute pipeline
    VkComputePipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.stage = computeShaderStageInfo;
    pipelineInfo.layout = pipelineLayout;
    
    if (vkCreateComputePipelines(device->getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &computePipeline) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create compute pipeline!");
    }
    
    vkDestroyShaderModule(device->getDevice(), computeShaderModule, nullptr);
    
    Logger::info("Compute pipeline created successfully");
}

void VulkanComputePipeline::createOutputImage(VkExtent2D extent) {
    Logger::info("Creating output image for compute shader");
    createOutputImageResources(extent);
}

void VulkanComputePipeline::createOutputImageResources(VkExtent2D extent) {
    // Create image
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = extent.width;
    imageInfo.extent.height = extent.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    if (vkCreateImage(device->getDevice(), &imageInfo, nullptr, &outputImage) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create compute output image!");
    }
    
    // Allocate memory
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device->getDevice(), outputImage, &memRequirements);
    
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = device->findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    
    if (vkAllocateMemory(device->getDevice(), &allocInfo, nullptr, &outputImageMemory) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate compute output image memory!");
    }
    
    vkBindImageMemory(device->getDevice(), outputImage, outputImageMemory, 0);
    
    // Create image view
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = outputImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;
    
    if (vkCreateImageView(device->getDevice(), &viewInfo, nullptr, &outputImageView) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create compute output image view!");
    }
    
    Logger::info("Compute output image created successfully");
}

std::vector<char> VulkanComputePipeline::readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filename);
    }
    
    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);
    
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();
    
    return buffer;
}

VkShaderModule VulkanComputePipeline::createShaderModule(const std::vector<char>& code) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
    
    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device->getDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create compute shader module!");
    }
    
    return shaderModule;
} 