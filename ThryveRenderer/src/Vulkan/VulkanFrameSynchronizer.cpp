//
// Created by kprie on 14.03.2024.
//

#include "Vulkan/VulkanFrameSynchronizer.h"

#include <iostream>

#include "utils/VkDebugUtils.h"

VulkanFrameSynchronizer::VulkanFrameSynchronizer(const VkDevice device, const uint32_t maxFramesInFlight, const VkQueue graphicsQueue) : m_device(device), m_maxFramesInFlight(maxFramesInFlight), m_GraphicsQueue(graphicsQueue){
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
        VK_CALL(vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &syncObject.imageAvailableSemaphore));
        VK_CALL(vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &syncObject.renderFinishedSemaphore));
        VK_CALL(vkCreateFence(m_device, &fenceInfo, nullptr, &syncObject.inFlightFence));
    }
}

void VulkanFrameSynchronizer::DestroySyncObjects() {
    for (auto& syncObject : m_syncObjects) {
        vkDestroySemaphore(m_device, syncObject.imageAvailableSemaphore, nullptr);
        vkDestroySemaphore(m_device, syncObject.renderFinishedSemaphore, nullptr);
        vkDestroyFence(m_device, syncObject.inFlightFence, nullptr);
    }
}

//TODO Check for redundancy with VulkanCommandBuffer::Submit()
bool VulkanFrameSynchronizer::SubmitCommandBuffers(const VkCommandBuffer* commandBuffers, uint32_t currentFrame, uint32_t imageIndex) {
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {m_syncObjects[currentFrame].imageAvailableSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = commandBuffers; // Assuming commandBuffers is a pointer to a single command buffer

    VkSemaphore signalSemaphores[] = {m_syncObjects[currentFrame].renderFinishedSemaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    // Reset the in-flight fence before submitting the command buffer
    vkResetFences(m_device, 1, &m_syncObjects[currentFrame].inFlightFence);

    if (vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, m_syncObjects[currentFrame].inFlightFence) != VK_SUCCESS) {
        std::cerr << "Failed to submit draw command buffer!" << std::endl;
        return false;
    }

    return true;
}

void VulkanFrameSynchronizer::ResetFences(uint32_t currentFrame) {
    auto fence = m_syncObjects[currentFrame].inFlightFence;
    VK_CALL(vkResetFences(m_device, 1, &fence));
}

bool VulkanFrameSynchronizer::WaitForFences(uint32_t currentFrame) {
    auto fence = m_syncObjects[currentFrame].inFlightFence;
    VkResult result = vkWaitForFences(m_device, 1, &fence, VK_TRUE, UINT64_MAX);
    return result == VK_SUCCESS;
}

uint32_t VulkanFrameSynchronizer::AdvanceFrame(uint32_t currentFrame) {
    // Advance the frame index, wrapping around as needed based on maxFramesInFlight
    return (currentFrame + 1) % m_maxFramesInFlight;
}
