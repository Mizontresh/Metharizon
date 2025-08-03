#include "PhysicsComputeRenderer.h"
#include "vulkan/VulkanDevice.h"
#include "vulkan/VulkanCommandPool.h"
#include "core/Logger.h"
#include <glm/glm.hpp>
#include <stdexcept>

struct BodyInit {
    glm::vec4 posRad;
    glm::vec4 velMas;
    glm::vec4 quat;
    glm::vec4 angVel;
};

PhysicsComputeRenderer::PhysicsComputeRenderer(VulkanDevice* device, VulkanCommandPool* commandPool)
    : device(device), commandPool(commandPool) {
    Logger::info("Creating Physics Compute Renderer");

    pipeline = new VulkanPhysicsPipeline(device);

    createStateBuffer();
    createDescriptorSet();
    createCommandBuffer();
}

PhysicsComputeRenderer::~PhysicsComputeRenderer() {
    Logger::info("Destroying Physics Compute Renderer");

    vkDeviceWaitIdle(device->getDevice());

    if (commandBuffer != VK_NULL_HANDLE) {
        vkFreeCommandBuffers(device->getDevice(), commandPool->getCommandPool(), 1, &commandBuffer);
    }
    if (descriptorPool != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(device->getDevice(), descriptorPool, nullptr);
    }
    if (stateBuffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(device->getDevice(), stateBuffer, nullptr);
    }
    if (stateBufferMemory != VK_NULL_HANDLE) {
        vkFreeMemory(device->getDevice(), stateBufferMemory, nullptr);
    }

    delete pipeline;
}

void PhysicsComputeRenderer::createStateBuffer() {
    // Two bodies → 2 * vec4 * 4 = 128 bytes
    VkDeviceSize bufferSize = sizeof(glm::vec4) * 8;

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size  = bufferSize;
    bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device->getDevice(), &bufferInfo, nullptr, &stateBuffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create state buffer");
    }

    VkMemoryRequirements memReq;
    vkGetBufferMemoryRequirements(device->getDevice(), stateBuffer, &memReq);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReq.size;
    allocInfo.memoryTypeIndex = device->findMemoryType(memReq.memoryTypeBits,
                                                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    if (vkAllocateMemory(device->getDevice(), &allocInfo, nullptr, &stateBufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate state buffer memory");
    }

    vkBindBufferMemory(device->getDevice(), stateBuffer, stateBufferMemory, 0);

    // Initial data
    BodyInit bodies[2] = {};
    // Marble
    bodies[0].posRad       = glm::vec4(0.0f, 1.5f, -5.0f, 0.3f);
    bodies[0].velMas       = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    bodies[0].quat  = glm::vec4(0.0f,0.0f,0.0f,1.0f);
    bodies[0].angVel = glm::vec4(0.0f);

    // Mandelbulb
    bodies[1].posRad = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f); // radius 1 m for quicker spin
    bodies[1].velMas = glm::vec4(0.0f, 0.0f, 0.0f, 2.0f);
    bodies[1].quat   = glm::vec4(0.0f,0.0f,0.0f,1.0f);
    bodies[1].angVel = glm::vec4(0.0f);

    void* data;
    vkMapMemory(device->getDevice(), stateBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, bodies, bufferSize);
    vkUnmapMemory(device->getDevice(), stateBufferMemory);
}

void PhysicsComputeRenderer::createDescriptorSet() {
    // pool
    VkDescriptorPoolSize poolSize{};
    poolSize.type            = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSize.descriptorCount = 1;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes    = &poolSize;
    poolInfo.maxSets       = 1;

    if (vkCreateDescriptorPool(device->getDevice(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create physics descriptor pool");
    }

    // allocate set
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = 1;
    VkDescriptorSetLayout setLayout = pipeline->getDescriptorSetLayout();
    allocInfo.pSetLayouts = &setLayout;

    if (vkAllocateDescriptorSets(device->getDevice(), &allocInfo, &descriptorSet) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate physics descriptor set");
    }

    VkDescriptorBufferInfo bufInfo{};
    bufInfo.buffer = stateBuffer;
    bufInfo.offset = 0;
    bufInfo.range  = VK_WHOLE_SIZE;

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = descriptorSet;
    write.dstBinding = 0;
    write.descriptorCount = 1;
    write.descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    write.pBufferInfo     = &bufInfo;

    vkUpdateDescriptorSets(device->getDevice(), 1, &write, 0, nullptr);
}

void PhysicsComputeRenderer::createCommandBuffer() {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool        = commandPool->getCommandPool();
    allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(device->getDevice(), &allocInfo, &commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate physics command buffer");
    }
}

void PhysicsComputeRenderer::updatePushConstants(float dt, float G, float restitution) {
    // Re-record command buffer each frame with updated constants
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkResetCommandBuffer(commandBuffer, 0);
    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->getPipeline());
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                            pipeline->getPipelineLayout(), 0, 1, &descriptorSet, 0, nullptr);

    float pcs[3] = { dt, G, restitution };
    vkCmdPushConstants(commandBuffer, pipeline->getPipelineLayout(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(pcs), pcs);

    vkCmdDispatch(commandBuffer, 1, 1, 1);

    vkEndCommandBuffer(commandBuffer);
}

void PhysicsComputeRenderer::dispatch(VkQueue queue) {
    // submit recorded command buffer and wait (could use fence for async)
    VkSubmitInfo submit{};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(queue, 1, &submit, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);
}
