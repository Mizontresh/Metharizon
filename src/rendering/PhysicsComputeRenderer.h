#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include "vulkan/VulkanPhysicsPipeline.h"

class VulkanDevice;
class VulkanCommandPool;

class PhysicsComputeRenderer {
public:
    PhysicsComputeRenderer(VulkanDevice* device, VulkanCommandPool* commandPool);
    ~PhysicsComputeRenderer();

    void updatePushConstants(float dt, float G, float restitution);

    // Call every frame to dispatch the physics step. For now we use a
    // single, long-lived command buffer that we re-record each frame.
    void dispatch(VkQueue queue);

    // Expose the GPU buffer so the app can read/write initial state or
    // bind it elsewhere (e.g. to the render pipeline later).
    VkBuffer getStateBuffer() const { return stateBuffer; }
    VkDeviceMemory getStateBufferMemory() const { return stateBufferMemory; }

private:
    VulkanDevice* device;
    VulkanCommandPool* commandPool;

    VulkanPhysicsPipeline* pipeline;

    VkBuffer stateBuffer = VK_NULL_HANDLE;
    VkDeviceMemory stateBufferMemory = VK_NULL_HANDLE;
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    VkDescriptorSet  descriptorSet  = VK_NULL_HANDLE;

    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;

    void createStateBuffer();
    void createDescriptorSet();
    void createCommandBuffer();
};
