#include "RaymarchRenderer.h"
#include "vulkan/VulkanDevice.h"
#include "vulkan/VulkanPipeline.h"
#include "vulkan/VulkanCommandPool.h"
#include "vulkan/VulkanSwapChain.h"
#include "vulkan/VulkanRenderPass.h"
#include "vulkan/VulkanDescriptorPool.h"
#include "core/Logger.h"
#include <stdexcept>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

RaymarchRenderer::RaymarchRenderer(VulkanDevice* device, VulkanPipeline* pipeline, VulkanCommandPool* commandPool)
    : device(device), pipeline(pipeline), commandPool(commandPool), renderPass(nullptr), swapChain(nullptr),
      uniformBuffer(VK_NULL_HANDLE), uniformBufferMemory(VK_NULL_HANDLE), descriptorSet(VK_NULL_HANDLE),
      cameraPos(0.0f, 0.0f, -3.0f), time(0.0f), viewMatrix(1.0f), aspectRatio(16.0f/9.0f) {
    Logger::info("Creating Raymarch Renderer");
    
    // Get references to swap chain and render pass from the pipeline
    // We'll need to pass these as parameters or access them differently
    // For now, we'll create a basic setup
    createCommandBuffers();
}

RaymarchRenderer::~RaymarchRenderer() {
    Logger::info("Destroying Raymarch Renderer");
    cleanup();
}

void RaymarchRenderer::cleanupFramebuffers() {
    for (auto framebuffer : swapChainFramebuffers) {
        vkDestroyFramebuffer(device->getDevice(), framebuffer, nullptr);
    }
    swapChainFramebuffers.clear();
}

void RaymarchRenderer::createFramebuffers(VulkanSwapChain* swapChain, VulkanRenderPass* renderPass) {
    Logger::info("Creating framebuffers");
    
    // Clean up old framebuffers first
    cleanupFramebuffers();
    
    this->renderPass = renderPass;
    this->swapChain = swapChain;
    
    auto swapChainImageViews = swapChain->getSwapChainImageViews();
    swapChainFramebuffers.resize(swapChainImageViews.size());
    
    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
        createFramebuffer(swapChainImageViews[i], swapChainFramebuffers[i], swapChain->getSwapChainExtent());
    }
    
    Logger::info("Created " + std::to_string(swapChainFramebuffers.size()) + " framebuffers");
}

void RaymarchRenderer::createFramebuffer(VkImageView imageView, VkFramebuffer& framebuffer, VkExtent2D extent) {
    VkImageView attachments[] = {imageView};
    
    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass->getRenderPass();
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = attachments;
    framebufferInfo.width = extent.width;
    framebufferInfo.height = extent.height;
    framebufferInfo.layers = 1;
    
    if (vkCreateFramebuffer(device->getDevice(), &framebufferInfo, nullptr, &framebuffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create framebuffer!");
    }
}

void RaymarchRenderer::createCommandBuffers() {
    Logger::info("Creating command buffers");
    
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

void RaymarchRenderer::createUniformBuffer() {
    // Create a simple uniform buffer with camera and time data
    struct UniformBufferObject {
        glm::mat4 view;
        glm::mat4 proj;
        glm::vec3 cameraPos;
        float time;
    };
    
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);
    
    device->createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                       uniformBuffer, uniformBufferMemory);
    
    Logger::info("Created uniform buffer");
}

void RaymarchRenderer::createDescriptorSet() {
    // For now, we'll skip descriptor set creation since we don't have access to the descriptor pool
    // The uniform buffer will still be updated, but we won't bind it to avoid validation errors
    Logger::info("Skipping descriptor set creation - uniform buffer will be updated but not bound");
}

void RaymarchRenderer::updateUniformBuffer(const glm::mat4& view, const glm::mat4& proj, const glm::vec3& cameraPos, float time) {
    struct UniformBufferObject {
        glm::mat4 view;
        glm::mat4 proj;
        glm::vec3 cameraPos;
        float time;
    };
    
    UniformBufferObject ubo{};
    ubo.view = view;
    ubo.proj = proj;
    ubo.cameraPos = cameraPos;
    ubo.time = time;
    
    void* data;
    vkMapMemory(device->getDevice(), uniformBufferMemory, 0, sizeof(ubo), 0, &data);
    memcpy(data, &ubo, sizeof(ubo));
    vkUnmapMemory(device->getDevice(), uniformBufferMemory);
}

void RaymarchRenderer::updateCameraData(const glm::vec3& cameraPos, float time, const glm::mat4& viewMatrix, float aspectRatio) {
    // Store camera data for use in command buffer recording
    this->cameraPos = cameraPos;
    this->time = time;
    this->viewMatrix = viewMatrix;
    this->aspectRatio = aspectRatio;
}

void RaymarchRenderer::recordCommandBuffer(uint32_t imageIndex, size_t currentFrame) {
    // Safety checks
    if (!device || !pipeline || !renderPass || !swapChain) {
        throw std::runtime_error("Renderer components are null!");
    }
    
    if (currentFrame >= commandBuffers.size()) {
        throw std::runtime_error("Frame index out of bounds!");
    }
    
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    
    if (vkBeginCommandBuffer(commandBuffers[currentFrame], &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("Failed to begin recording command buffer!");
    }
    
    // Safety check: ensure we have valid swap chain and framebuffers
    if (imageIndex >= swapChainFramebuffers.size()) {
        throw std::runtime_error("Invalid framebuffer index!");
    }
    
    // Begin render pass
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass->getRenderPass();
    renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    
    // Get the actual extent from the swap chain
    VkExtent2D extent = swapChain->getSwapChainExtent();
    renderPassInfo.renderArea.extent = extent;
    
    VkClearValue clearColor = {{{0.0f, 0.0f, 0.5f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;
    
    vkCmdBeginRenderPass(commandBuffers[currentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    
    // Bind the graphics pipeline
    vkCmdBindPipeline(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getPipeline());
    
    // Set dynamic viewport and scissor to match the actual window size
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapChain->getSwapChainExtent().width);
    viewport.height = static_cast<float>(swapChain->getSwapChainExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffers[currentFrame], 0, 1, &viewport);
    
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapChain->getSwapChainExtent();
    vkCmdSetScissor(commandBuffers[currentFrame], 0, 1, &scissor);
    
    // Push camera data to shader
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
    
    vkCmdPushConstants(commandBuffers[currentFrame], pipeline->getPipelineLayout(), 
                      VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstants), &pushConstants);
    
    // Draw a full-screen triangle (3 vertices)
    // This will trigger the vertex shader which generates a full-screen triangle
    // and the fragment shader which performs raymarching
    vkCmdDraw(commandBuffers[currentFrame], 3, 1, 0, 0);
    
    vkCmdEndRenderPass(commandBuffers[currentFrame]);
    
    if (vkEndCommandBuffer(commandBuffers[currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("Failed to record command buffer!");
    }
}

VkCommandBuffer RaymarchRenderer::getCommandBuffer(size_t frameIndex) {
    if (frameIndex >= commandBuffers.size()) {
        throw std::runtime_error("Frame index out of bounds!");
    }
    return commandBuffers[frameIndex];
}

void RaymarchRenderer::cleanup() {
    cleanupFramebuffers();
    
    if (!commandBuffers.empty()) {
        vkFreeCommandBuffers(device->getDevice(), commandPool->getCommandPool(), 
                           static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
        commandBuffers.clear();
    }
} 