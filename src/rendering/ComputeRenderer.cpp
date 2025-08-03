#include "ComputeRenderer.h"
#include "vulkan/VulkanDevice.h"
#include "vulkan/VulkanCommandPool.h"
#include "vulkan/VulkanSwapChain.h"
#include "vulkan/VulkanComputePipeline.h"
#include "core/Logger.h"
#include <stdexcept>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

ComputeRenderer::ComputeRenderer(VulkanDevice* device, VulkanCommandPool* commandPool)
    : device(device), commandPool(commandPool), computePipeline(nullptr),
      descriptorSet(VK_NULL_HANDLE), descriptorPool(VK_NULL_HANDLE),
      cameraPos(0.0f, 0.0f, -3.0f), time(0.0f), viewMatrix(1.0f), aspectRatio(16.0f/9.0f) {
    Logger::info("Creating Compute Renderer");
    
    // Create compute pipeline
    computePipeline = new VulkanComputePipeline(device);
    
    createCommandBuffers();
}

ComputeRenderer::~ComputeRenderer() {
    Logger::info("Destroying Compute Renderer");
    cleanup();
}

void ComputeRenderer::createCommandBuffers() {
    Logger::info("Creating compute command buffers");
    
    commandBuffers.resize(2); // MAX_FRAMES_IN_FLIGHT
    
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool->getCommandPool();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();
    
    if (vkAllocateCommandBuffers(device->getDevice(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate command buffers!");
    }
    
    Logger::info("Created " + std::to_string(commandBuffers.size()) + " command buffers");
}

void ComputeRenderer::createDescriptorSet() {
    Logger::info("Creating compute descriptor set");
    
    // Create descriptor pool
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    poolSize.descriptorCount = 1;
    
    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = 1;
    
    if (vkCreateDescriptorPool(device->getDevice(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor pool!");
    }
    
    // Allocate descriptor set
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = 1;
    VkDescriptorSetLayout descriptorSetLayout = computePipeline->getDescriptorSetLayout();
    allocInfo.pSetLayouts = &descriptorSetLayout;
    
    if (vkAllocateDescriptorSets(device->getDevice(), &allocInfo, &descriptorSet) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate descriptor set!");
    }
    
    // Update descriptor set
    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    imageInfo.imageView = computePipeline->getOutputImageView();
    imageInfo.sampler = VK_NULL_HANDLE;
    
    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = descriptorSet;
    descriptorWrite.dstBinding = 1;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &imageInfo;
    
    vkUpdateDescriptorSets(device->getDevice(), 1, &descriptorWrite, 0, nullptr);
    
    Logger::info("Compute descriptor set created successfully");
}

void ComputeRenderer::createOutputImage(VulkanSwapChain* swapChain) {
    Logger::info("Creating output image for compute shader");
    
    imageExtent = swapChain->getSwapChainExtent();
    computePipeline->createOutputImage(imageExtent);
    
    Logger::info("Output image created successfully");
}

void ComputeRenderer::updateCameraData(const glm::vec3& cameraPos, float time, const glm::mat4& viewMatrix, float aspectRatio) {
    this->cameraPos = cameraPos;
    this->time = time;
    this->viewMatrix = viewMatrix;
    this->aspectRatio = aspectRatio;
}

void ComputeRenderer::recordCommandBuffer(uint32_t imageIndex, size_t currentFrame, VkImage swapChainImage) {
    if (currentFrame >= commandBuffers.size()) {
        throw std::runtime_error("Frame index out of bounds!");
    }
    
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    
    if (vkBeginCommandBuffer(commandBuffers[currentFrame], &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("Failed to begin recording command buffer!");
    }
    
    // Transition output image to general layout for compute shader
    transitionImageLayout(computePipeline->getOutputImage(), VK_FORMAT_R8G8B8A8_UNORM, 
                        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
    
    // Bind compute pipeline
    vkCmdBindPipeline(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_COMPUTE, computePipeline->getPipeline());
    
    // Bind descriptor set
    vkCmdBindDescriptorSets(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_COMPUTE, 
                           computePipeline->getPipelineLayout(), 0, 1, &descriptorSet, 0, nullptr);
    
    // Push constants
    struct PushConstants {
        glm::vec3 cameraPos;
        float time;
        glm::mat4 viewMatrix;
        float aspectRatio;
    } pushConstants;
    
    pushConstants.cameraPos = cameraPos;
    pushConstants.time = time;
    pushConstants.viewMatrix = viewMatrix;
    pushConstants.aspectRatio = aspectRatio;
    
    vkCmdPushConstants(commandBuffers[currentFrame], computePipeline->getPipelineLayout(), 
                      VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(PushConstants), &pushConstants);
    
    // Dispatch compute shader
    uint32_t groupCountX = (imageExtent.width + 15) / 16;
    uint32_t groupCountY = (imageExtent.height + 15) / 16;
    vkCmdDispatch(commandBuffers[currentFrame], groupCountX, groupCountY, 1);
    
    // Copy compute output to swap chain image
    if (swapChainImage != VK_NULL_HANDLE) {
        // Transition swap chain image to transfer dst layout
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = swapChainImage;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        
        vkCmdPipelineBarrier(
            commandBuffers[currentFrame],
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );
        
        // Copy from compute output to swap chain image
        VkImageCopy copyRegion{};
        copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copyRegion.srcSubresource.mipLevel = 0;
        copyRegion.srcSubresource.baseArrayLayer = 0;
        copyRegion.srcSubresource.layerCount = 1;
        copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copyRegion.dstSubresource.mipLevel = 0;
        copyRegion.dstSubresource.baseArrayLayer = 0;
        copyRegion.dstSubresource.layerCount = 1;
        copyRegion.extent = {imageExtent.width, imageExtent.height, 1};
        
        vkCmdCopyImage(
            commandBuffers[currentFrame],
            computePipeline->getOutputImage(), VK_IMAGE_LAYOUT_GENERAL,
            swapChainImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1, &copyRegion
        );
        
        // Transition swap chain image to present layout
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = 0;
        
        vkCmdPipelineBarrier(
            commandBuffers[currentFrame],
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );
    }
    
    if (vkEndCommandBuffer(commandBuffers[currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("Failed to record command buffer!");
    }
}

VkCommandBuffer ComputeRenderer::getCommandBuffer(size_t frameIndex) {
    if (frameIndex >= commandBuffers.size()) {
        throw std::runtime_error("Frame index out of bounds!");
    }
    return commandBuffers[frameIndex];
}

void ComputeRenderer::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
    VkCommandBuffer commandBuffer = commandPool->beginSingleTimeCommands();
    
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    
    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;
    
    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_GENERAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    } else {
        throw std::invalid_argument("Unsupported layout transition!");
    }
    
    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );
    
    commandPool->endSingleTimeCommands(commandBuffer);
}

void ComputeRenderer::cleanup() {
    if (!commandBuffers.empty()) {
        vkFreeCommandBuffers(device->getDevice(), commandPool->getCommandPool(), 
                           static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
        commandBuffers.clear();
    }
    
    if (descriptorPool != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(device->getDevice(), descriptorPool, nullptr);
        descriptorPool = VK_NULL_HANDLE;
    }
    
    if (computePipeline != nullptr) {
        delete computePipeline;
        computePipeline = nullptr;
    }
} 