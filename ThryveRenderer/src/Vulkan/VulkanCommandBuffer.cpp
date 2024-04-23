//
// Created by kprie on 14.03.2024.
//

#include "Vulkan/VulkanCommandBuffer.h"
#include <mutex>

#include "Vulkan/VulkanContext.h"
#include "utils/VkDebugUtils.h"

VulkanCommandBuffer::VulkanCommandBuffer(const VkCommandPool commandPool): m_commandPool(commandPool) {
    auto _deviceSelector = Thryve::Core::App::Get().GetWindow()->GetRenderContext().As<Thryve::Rendering::VulkanContext>()->GetDevice();
    m_device = _deviceSelector->GetLogicalDevice();
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

void VulkanCommandBuffer::Begin(const VkCommandBuffer commandBuffer
                                , const VkCommandBufferUsageFlags usageFlags) const {
    std::lock_guard<std::mutex> lock(m_mutex);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = usageFlags;

    VK_CALL(vkBeginCommandBuffer(commandBuffer, &beginInfo));
}

void VulkanCommandBuffer::End(const VkCommandBuffer commandBuffer) const {
    std::lock_guard<std::mutex> lock(m_mutex);

    VK_CALL(vkEndCommandBuffer(commandBuffer));
}

void VulkanCommandBuffer::Submit(const VkCommandBuffer commandBuffer, const VkQueue queue
                                 , const bool waitForCompletion) const {
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
