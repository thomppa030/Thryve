//
// Created by kprie on 14.03.2024.
//

#include "Vulkan/VulkanCommandBuffer.h"
#include <mutex>
#include "utils/VkDebugUtils.h"

VulkanCommandBuffer::VulkanCommandBuffer(const VkDevice device, const VkCommandPool commandPool): m_device(device), m_commandPool(commandPool) {
}

VkCommandBuffer VulkanCommandBuffer::Allocate(bool bBegingRecording) {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = m_commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    VK_CALL(vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer));

    if (bBegingRecording) {
        Begin(commandBuffer);
    }

    return commandBuffer;
}

void VulkanCommandBuffer::Begin(VkCommandBuffer commandBuffer
                                                    , VkCommandBufferUsageFlags usageFlags) {
    std::lock_guard<std::mutex> lock(m_mutex);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = usageFlags;

    VK_CALL(vkBeginCommandBuffer(commandBuffer, &beginInfo));
}

void VulkanCommandBuffer::End(VkCommandBuffer commandBuffer) {
    std::lock_guard<std::mutex> lock(m_mutex);

    VK_CALL(vkEndCommandBuffer(commandBuffer));
}

void VulkanCommandBuffer::Submit(VkCommandBuffer commandBuffer, VkQueue queue
    , bool waitForCompletion){
    std::lock_guard<std::mutex> lock(m_mutex);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    if (waitForCompletion) {
        VkFence fence;
        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        VK_CALL(vkCreateFence(m_device, &fenceInfo, nullptr, &fence));

        VK_CALL(vkQueueSubmit(queue, 1, &submitInfo, fence));

        VK_CALL(vkWaitForFences(m_device, 1, &fence, VK_TRUE, UINT64_MAX));
        vkDestroyFence(m_device, fence, nullptr);
    } else {
        VK_CALL(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));
    }
}

void VulkanCommandBuffer::Free(const VkCommandBuffer& commandBuffer) const {
    if (commandBuffer != VK_NULL_HANDLE) {
        vkFreeCommandBuffers(m_device, m_commandPool, 1, &commandBuffer);
    }
}
