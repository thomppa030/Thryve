//
// Created by kprie on 15.07.2024.
//

#include "VulkanBuffer.h"

#include <glm/glm.hpp>
#include "Core/App.h"
#include "VulkanContext.h"
#include "VulkanDeviceSelector.h"
#include "utils/VkDebugUtils.h"


namespace Thryve::Rendering {

    VulkanBuffer::VulkanBuffer(VmaAllocator vmaAllocator, VkDeviceSize size, VkBufferUsageFlags usage,
                               VulkanBuffer* actualBuffer, const std::string& name)
                                   : m_Allocator{vmaAllocator}, m_Size{size}, m_ActualBufferIfStaging{actualBuffer}, m_Usage{usage}
    {
        VkBufferCreateInfo _bufferCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .size = m_Size,
            .usage = m_Usage | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = {},
            .pQueueFamilyIndices = {},
        };

        m_AllocCreateInfo = {VMA_ALLOCATION_CREATE_MAPPED_BIT, VMA_MEMORY_USAGE_CPU_ONLY};

        VK_CALL(
            vmaCreateBuffer(m_Allocator, &_bufferCreateInfo, &m_AllocCreateInfo, &m_Buffer, &m_Allocation, nullptr));

        vmaGetAllocationInfo(m_Allocator, m_Allocation, &m_AllocationInfo);
    }

    VulkanBuffer::VulkanBuffer(VmaAllocator vmaAllocator, const VkBufferCreateInfo& createInfo,
                               const VmaAllocationCreateInfo& allocInfo, const std::string& name)
                                   : m_Allocator{vmaAllocator}, m_Size{createInfo.size}, m_Usage{createInfo.usage}, m_AllocCreateInfo{allocInfo}
    {
        VK_CALL(vmaCreateBuffer(m_Allocator, &createInfo, &allocInfo, &m_Buffer, &m_Allocation, nullptr));

        vmaGetAllocationInfo(m_Allocator, m_Allocation, &m_AllocationInfo);
    }

    VulkanBuffer::~VulkanBuffer()
    {
        if (m_MappedMemory)
        {
            vmaUnmapMemory(m_Allocator, m_Allocation);
        }

        for (auto& [bufferviewFormat, bufferView]: m_BufferViews)
        {
            vkDestroyBufferView(VulkanContext::GetCurrentDevice()->GetLogicalDevice(), bufferView, nullptr);
        }

        vmaDestroyBuffer(m_Allocator, m_Buffer, m_Allocation);
    }

    VkDeviceSize VulkanBuffer::GetSize() const
    {
        return m_Size;
    }

    void VulkanBuffer::Upload(const VkDeviceSize offset) const
    {
        Upload(offset, m_Size);
    }

    void VulkanBuffer::Upload(VkDeviceSize offset, VkDeviceSize size) const
    {
        VK_CALL(vmaFlushAllocation(m_Allocator, m_Allocation, offset, m_Size));
    }

    void VulkanBuffer::UploadStagingBufferToGPU(const VkCommandBuffer& commandBuffer, uint64_t srcOffset,
                                                uint64_t dstOffset) const
    {
        VkBufferCopy region = {
            .srcOffset = srcOffset,
            .dstOffset = dstOffset,
            .size = m_Size};

        vkCmdCopyBuffer(commandBuffer, GetBuffer(), m_ActualBufferIfStaging->GetBuffer(), 1, &region);
    }

    void VulkanBuffer::CopyDataToBuffer(const void* data, size_t size) const
    {
        if (!m_MappedMemory)
        {
            VK_CALL(vmaMapMemory(m_Allocator, m_Allocation, &m_MappedMemory));
        }
        memcpy(m_MappedMemory, data, size);
    }

    VkDeviceAddress VulkanBuffer::GetDeviceAddress() const
    {
        if (m_ActualBufferIfStaging)
        {
            return m_ActualBufferIfStaging->GetDeviceAddress();
        }

#if defined(VK_KHR_buffer_device_address) && defined(_WIN32)
        if (!m_BufferDeviceAddress)
        {
            const VkBufferDeviceAddressInfo _bdAddressInfo = {
                .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
                .buffer = m_Buffer,
            };
            m_BufferDeviceAddress =
                vkGetBufferDeviceAddress(VulkanContext::GetCurrentDevice()->GetLogicalDevice(), &_bdAddressInfo);
        }
        return m_BufferDeviceAddress;
#else
        return 0;
#endif
    }

    VkBufferView VulkanBuffer::RequestBufferView(const VkFormat viewFormat)
    {
        VkBufferViewCreateInfo _bufferViewCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO,
        .flags = 0,
        .buffer = GetBuffer(),
        .format = viewFormat,
        .offset =  0,
        .range = m_Size,
        };

        VkBufferView _bufferView;
        VK_CALL(vkCreateBufferView(VulkanContext::GetCurrentDevice()->GetLogicalDevice(), &_bufferViewCreateInfo,
                                   nullptr, &_bufferView));

        m_BufferViews[viewFormat] = _bufferView;
        return _bufferView;
    }
} // Rendering