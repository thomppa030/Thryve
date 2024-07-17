//
// Created by kprie on 15.07.2024.
//

#include "VulkanCommandQueueManager.h"

#include "spdlog/fmt/bundled/chrono.h"
#include "utils/VkDebugUtils.h"

namespace Thryve::Rendering {
    VulkanCommandQueueManager::VulkanCommandQueueManager(VkDevice device, uint32_t count,
                                                         uint32_t concurrentNumCommands, uint32_t queueFamilyIndex,
                                                         VkQueue queue, VkCommandPoolCreateFlags flags,
                                                         const std::string& name) :
        m_CommandsInFlight{concurrentNumCommands}, m_QueueFamilyIndex{queueFamilyIndex}, m_Queue{queue},
        m_Device{device}
    {
        m_Fences.reserve(m_CommandsInFlight);
        m_IsSubmitted.reserve(m_CommandsInFlight);
        m_BufferToDispose.reserve(m_CommandsInFlight);
        m_Deallocators.reserve(m_CommandsInFlight);

        const VkCommandPoolCreateInfo _commandPoolCreateInfo = {.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                                                                .flags = flags,
                                                                .queueFamilyIndex = m_QueueFamilyIndex};

        VK_CALL(vkCreateCommandPool(m_Device, &_commandPoolCreateInfo, nullptr, &m_CommandPool));

        const VkCommandBufferAllocateInfo _commandBufferAllocateInfo{.sType =
                                                                         VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                                                                     .commandPool = m_CommandPool,
                                                                     .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                                                                     .commandBufferCount = 1};

        for (size_t _i = 0; _i < count; ++_i)
        {
            VkCommandBuffer cmdBuffer;
            VK_CALL(vkAllocateCommandBuffers(m_Device, &_commandBufferAllocateInfo, &cmdBuffer));
            m_CommandBuffers.push_back(cmdBuffer);
        }

        for (size_t _i = 0; _i < m_CommandsInFlight; ++_i)
        {
            VkFence _fence;

            constexpr VkFenceCreateInfo _fenceCreateInfo = {
                .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                .flags = VK_FENCE_CREATE_SIGNALED_BIT,
            };

            VK_CALL(vkCreateFence(m_Device, &_fenceCreateInfo, nullptr, &_fence));

            m_Fences.push_back(std::move(_fence));
            m_IsSubmitted.push_back(false);
        }
    }

    VulkanCommandQueueManager::~VulkanCommandQueueManager()
    {
        DeAllocateResources();

        for (size_t _i = 0; _i < m_CommandsInFlight; ++_i)
        {
            vkDestroyFence(m_Device, m_Fences[_i], nullptr);
        }

        for (auto& _commandBuffer : m_CommandBuffers)
        {
            vkFreeCommandBuffers(m_Device, m_CommandPool, 1, &_commandBuffer);
        }

        vkDestroyCommandPool(m_Device, m_CommandPool, nullptr);
    }

    void VulkanCommandQueueManager::Submit(const VkSubmitInfo* submitInfo)
    {
        VK_CALL(vkResetFences(m_Device, 1, &m_Fences[m_CurrentFenceIndex]));
        VK_CALL(vkQueueSubmit(m_Queue, 1, submitInfo, m_Fences[m_CurrentFenceIndex]));
        m_IsSubmitted[m_CurrentFenceIndex] = true;
    }

    void VulkanCommandQueueManager::GoToNextCmdBuffer()
    {
        m_CurrentCommandBufferIndex =
            (m_CurrentCommandBufferIndex + 1) % static_cast<uint32_t>(m_CommandBuffers.size());
        m_CurrentFenceIndex = (m_CurrentFenceIndex + 1) % m_CommandsInFlight;
    }

    void VulkanCommandQueueManager::WaitUntilSubmitIsComplete()
    {
        if (m_IsSubmitted[m_CurrentFenceIndex])
        {
            return;
        }

        const auto _result = vkWaitForFences(m_Device, 1, &m_Fences[m_CurrentFenceIndex], true, UINT32_MAX);

        if (_result == VK_TIMEOUT)
        {
            std::cerr << "TimeOut" << std::endl;
            vkDeviceWaitIdle(m_Device);
        }

        m_IsSubmitted[m_CurrentFenceIndex] = false;
        m_BufferToDispose[m_CurrentFenceIndex].clear();
        DeAllocateResources();
    }

    void VulkanCommandQueueManager::WaitUntilAllSubmitsAreComplete()
    {
        for (size_t _index = 0; auto& _fence : m_Fences)
        {
            VK_CALL(vkWaitForFences(m_Device, 1, &_fence, true, UINT32_MAX));
            VK_CALL(vkResetFences(m_Device, 1, &_fence));
            m_IsSubmitted[_index++] = false;
        }
        m_BufferToDispose.clear();
        DeAllocateResources();
    }

    void VulkanCommandQueueManager::DisposeWhenSubmitCompletes(std::shared_ptr<VulkanBuffer> buffer)
    {
        m_BufferToDispose[m_CurrentFenceIndex].push_back(std::move(buffer));
    }

    void VulkanCommandQueueManager::DisposeWhenSubmitCompletes(std::function<void()>&& deallocator)
    {
        m_Deallocators[m_CurrentFenceIndex].push_back(std::move(deallocator));
    }

    VkCommandBuffer VulkanCommandQueueManager::GetCommandBufferToBegin()
    {
        VK_CALL(vkWaitForFences(m_Device, 1, &m_Fences[m_CurrentFenceIndex], true, UINT32_MAX));
        VK_CALL(vkResetCommandBuffer(m_CommandBuffers[m_CurrentCommandBufferIndex],
                                     VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT));

        const VkCommandBufferBeginInfo _info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        };

        VK_CALL(vkBeginCommandBuffer(m_CommandBuffers[m_CurrentCommandBufferIndex], &_info));

        return m_CommandBuffers[m_CurrentCommandBufferIndex];
    }

    VkCommandBuffer VulkanCommandQueueManager::GetCommandBuffer()
    {
        const VkCommandBufferAllocateInfo _commandBufferAllocateInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = m_CommandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
        };

        VkCommandBuffer cmdBuffer{VK_NULL_HANDLE};
        VK_CALL(vkAllocateCommandBuffers(m_Device, &_commandBufferAllocateInfo, &cmdBuffer));

        return cmdBuffer;
    }

    void VulkanCommandQueueManager::EndCommandBuffer(VkCommandBuffer cmdBuffer)
    {
        VK_CALL(vkEndCommandBuffer(cmdBuffer));
    }

    void VulkanCommandQueueManager::DeAllocateResources()
    {
        for (auto& _deAllocators : m_Deallocators)
        {
            for (auto& _deAllocator : _deAllocators)
            {
                _deAllocator();
            }
        }
    }
} // namespace Thryve::Rendering
