//
// Created by kprie on 14.03.2024.
//
#include "Vulkan/VulkanFrameSynchronizer.h"

#include <iostream>

#include "../../../GraphicsContext.h"
#include "Vulkan/VulkanContext.h"
#include "utils/VkDebugUtils.h"

VulkanFrameSynchronizer::VulkanFrameSynchronizer(const uint32_t maxFramesInFlight) : m_maxFramesInFlight(maxFramesInFlight){
    m_device = Thryve::Rendering::VulkanContext::GetCurrentDevice()->GetLogicalDevice();
    m_syncObjects.resize(maxFramesInFlight);
    CreateSyncObjects();
}

VulkanFrameSynchronizer::~VulkanFrameSynchronizer() {
    DestroySyncObjects();
}

const VulkanFrameSynchronizer::FrameSyncObjects & VulkanFrameSynchronizer::GetSyncObjects(const uint32_t frameIndex) const {
    return m_syncObjects[frameIndex % m_maxFramesInFlight];
}

void VulkanFrameSynchronizer::CreateSyncObjects() {
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (auto& syncObject : m_syncObjects) {
        VK_CALL(vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &syncObject.image_available_semaphore));
        VK_CALL(vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &syncObject.render_finished_semaphore));
        VK_CALL(vkCreateFence(m_device, &fenceInfo, nullptr, &syncObject.in_flight_fence));
    }
}

void VulkanFrameSynchronizer::DestroySyncObjects() const {
    for (auto& syncObject : m_syncObjects) {
        vkDestroySemaphore(m_device, syncObject.image_available_semaphore, nullptr);
        vkDestroySemaphore(m_device, syncObject.render_finished_semaphore, nullptr);
        vkDestroyFence(m_device, syncObject.in_flight_fence, nullptr);
    }
}

//TODO Check for redundancy with VulkanCommandBuffer::Submit()
bool VulkanFrameSynchronizer::SubmitCommandBuffers(const VkCommandBuffer* commandBuffers, uint32_t currentFrame, uint32_t imageIndex) const {
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    const VkSemaphore waitSemaphores[] = {m_syncObjects[currentFrame].image_available_semaphore};
    constexpr VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = commandBuffers; // Assuming commandBuffers is a pointer to a single command buffer

    const VkSemaphore signalSemaphores[] = {m_syncObjects[currentFrame].render_finished_semaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    // Reset the in-flight fence before submitting the command buffer
    vkResetFences(m_device, 1, &m_syncObjects[currentFrame].in_flight_fence);

    if (vkQueueSubmit(Thryve::Rendering::VulkanContext::GetCurrentDevice()->GetGraphicsQueue(), 1, &submitInfo, m_syncObjects[currentFrame].in_flight_fence) != VK_SUCCESS) {
        std::cerr << "Failed to submit draw command buffer!" << std::endl;
        return false;
    }

    return true;
}

void VulkanFrameSynchronizer::ResetFences(const uint32_t currentFrame) const {
    const auto fence = m_syncObjects[currentFrame].in_flight_fence;
    VK_CALL(vkResetFences(m_device, 1, &fence));
}

bool VulkanFrameSynchronizer::WaitForFences(const uint32_t currentFrame) const {
    const auto fence = m_syncObjects[currentFrame].in_flight_fence;
    const VkResult result = vkWaitForFences(m_device, 1, &fence, VK_TRUE, UINT64_MAX);
    return result == VK_SUCCESS;
}

uint32_t VulkanFrameSynchronizer::AdvanceFrame(const uint32_t currentFrame) const {
    // Advance the frame index, wrapping around as needed based on maxFramesInFlight
    return (currentFrame + 1) % m_maxFramesInFlight;
}
