//
// Created by kprie on 14.03.2024.
//
#pragma once
#include <mutex>
#include "pch.h"

class VulkanCommandBuffer {

public:
    VulkanCommandBuffer(VkDevice device, VkCommandPool commandPool);
    ~VulkanCommandBuffer() = default;

    VulkanCommandBuffer(const VulkanCommandBuffer&) = delete;
    VulkanCommandBuffer& operator=(const VulkanCommandBuffer&) = delete;
    VulkanCommandBuffer(const VulkanCommandBuffer&&) = delete;
    VulkanCommandBuffer& operator=(const VulkanCommandBuffer&&) = delete;

    VkCommandBuffer Allocate(bool bBeginRecording = false);

    // Begins recording commands into the specified command buffer.
    void Begin(VkCommandBuffer commandBuffer, VkCommandBufferUsageFlags usageFlags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT) const;

    // Ends recording commands into the specified command buffer.
    void End(VkCommandBuffer commandBuffer) const;

    // Submits a command buffer to a queue and optionally waits for execution to complete.
    void Submit(VkCommandBuffer commandBuffer, VkQueue queue, bool waitForCompletion = false) const;

    void Free(const VkCommandBuffer& commandBuffer) const;

private:
    VkDevice m_device;
    VkCommandPool m_commandPool;

    mutable std::mutex m_mutex;
};
