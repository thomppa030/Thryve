//
// Created by kprie on 15.07.2024.
//
#pragma once


#include <functional>

class VulkanCommandBuffer;

namespace Thryve::Rendering {
    class VulkanBuffer;

    class VulkanCommandQueueManager {
    public:
        explicit
        VulkanCommandQueueManager(VkDevice device, uint32_t count, uint32_t concurrentNumCommands,
                                  uint32_t queueFamilyIndex, VkQueue queue,
                                  VkCommandPoolCreateFlags flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                                  const std::string& name = "");

        ~VulkanCommandQueueManager();

        void Submit(const VkSubmitInfo* submitInfo);

        void GoToNextCmdBuffer();

        void WaitUntilSubmitIsComplete();
        void WaitUntilAllSubmitsAreComplete();

        void DisposeWhenSubmitCompletes(std::shared_ptr<VulkanBuffer> buffer);
        void DisposeWhenSubmitCompletes(std::function<void()>&& deallocator);

        VkCommandBuffer GetCommandBufferToBegin();

        VkCommandBuffer GetCommandBuffer();

        void EndCommandBuffer(VkCommandBuffer cmdBuffer);

        [[nodiscard]] uint32_t GetQueueFamilyIndex() const {return m_QueueFamilyIndex; }

    private:

        void DeAllocateResources();

        uint32_t m_CommandsInFlight = 2;
        uint32_t m_QueueFamilyIndex = 0;

        VkQueue m_Queue = VK_NULL_HANDLE;
        VkDevice m_Device = VK_NULL_HANDLE;
        VkCommandPool m_CommandPool = VK_NULL_HANDLE;

        std::vector<VkCommandBuffer> m_CommandBuffers;
        std::vector<VkFence> m_Fences;
        std::vector<bool> m_IsSubmitted;

        uint32_t m_CurrentFenceIndex = 0;
        uint32_t m_CurrentCommandBufferIndex = 0;

        // fenceIndex to list of buffers associated with that
        // fence that needs to be released
        std::vector<std::vector<std::shared_ptr<VulkanBuffer>>> m_BufferToDispose;
        std::vector<std::vector<std::function<void()>>> m_Deallocators;
    };
} // namespace Thryve::Rendering
