//
// Created by kprie on 14.03.2024.
//
#pragma once
#include "pch.h"

class VulkanFrameSynchronizer {
public:
    struct FrameSyncObjects {
        VkSemaphore imageAvailableSemaphore;
        VkSemaphore renderFinishedSemaphore;
        VkFence inFlightFence;
    };

    VulkanFrameSynchronizer(VkDevice device, uint32_t maxFramesInFlight, VkQueue graphicsQueue);
    ~VulkanFrameSynchronizer();

    bool WaitForFences(uint32_t currentFrame);

    uint32_t AdvanceFrame(uint32_t currentFrame);

    // Delete copy/move constructors and assignment operators
    VulkanFrameSynchronizer(const VulkanFrameSynchronizer&) = delete;
    VulkanFrameSynchronizer& operator=(const VulkanFrameSynchronizer&) = delete;
    VulkanFrameSynchronizer(VulkanFrameSynchronizer&&) = delete;
    VulkanFrameSynchronizer& operator=(VulkanFrameSynchronizer&&) = delete;

    [[nodiscard]] const FrameSyncObjects& GetSyncObjects(uint32_t frameIndex) const;
    bool SubmitCommandBuffers(const VkCommandBuffer *commandBuffers, uint32_t currentFrame, uint32_t imageIndex);

private:
    VkDevice m_device{};
    std::vector<FrameSyncObjects> m_syncObjects;
    uint32_t m_maxFramesInFlight{};
    VkQueue m_GraphicsQueue;

    void CreateSyncObjects();
    void DestroySyncObjects();


    void ResetFences(uint32_t currentFrame);
};