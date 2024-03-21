//
// Created by kprie on 14.03.2024.
//
#pragma once
#include "pch.h"

class VulkanFrameSynchronizer {
public:
    struct FrameSyncObjects {
        VkSemaphore image_available_semaphore;
        VkSemaphore render_finished_semaphore;
        VkFence in_flight_fence;
    };

    VulkanFrameSynchronizer(uint32_t maxFramesInFlight);
    ~VulkanFrameSynchronizer();

    [[nodiscard]] bool WaitForFences(uint32_t currentFrame) const;

    [[nodiscard]] uint32_t AdvanceFrame(uint32_t currentFrame) const;

    // Delete copy/move constructors and assignment operators
    VulkanFrameSynchronizer(const VulkanFrameSynchronizer&) = delete;
    VulkanFrameSynchronizer& operator=(const VulkanFrameSynchronizer&) = delete;
    VulkanFrameSynchronizer(VulkanFrameSynchronizer&&) = delete;
    VulkanFrameSynchronizer& operator=(VulkanFrameSynchronizer&&) = delete;

    [[nodiscard]] const FrameSyncObjects& GetSyncObjects(uint32_t frameIndex) const;
    bool SubmitCommandBuffers(const VkCommandBuffer *commandBuffers, uint32_t currentFrame, uint32_t imageIndex) const;

private:
    VkDevice m_device{};
    std::vector<FrameSyncObjects> m_syncObjects;
    uint32_t m_maxFramesInFlight{};

    void CreateSyncObjects();
    void DestroySyncObjects() const;

    void ResetFences(uint32_t currentFrame) const;
};