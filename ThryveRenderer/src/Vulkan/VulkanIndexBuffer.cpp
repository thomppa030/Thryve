//
// Created by kprie on 15.03.2024.
//

#include "Vulkan/VulkanIndexBuffer.h"

#include "Vulkan/VulkanContext.h"
#include "utils/VulkanBufferUtils.h"

namespace Thryve::Rendering {
    VulkanIndexBuffer::VulkanIndexBuffer(const VkCommandPool commandPool) :
        m_commandPool(commandPool), m_indexBuffer{nullptr}, m_indexBufferMemory{nullptr}
    {
        m_device = VulkanContext::GetCurrentDevice()->GetLogicalDevice();
        m_physicalDevice = VulkanContext::GetCurrentDevice()->GetPhysicalDevice();
    }

    VulkanIndexBuffer::~VulkanIndexBuffer()
    {
        if (m_indexBuffer != VK_NULL_HANDLE)
        {
            vkDestroyBuffer(m_device, m_indexBuffer, nullptr);
            m_indexBuffer = VK_NULL_HANDLE;
        }
        if (m_indexBufferMemory != VK_NULL_HANDLE)
        {
            vkFreeMemory(m_device, m_indexBufferMemory, nullptr);
        }
    }

    void VulkanIndexBuffer::Create(const std::vector<uint32_t> &indices)
    {
        m_indexCount = indices.size();
        const VkDeviceSize _bufferSize = sizeof(indices[0]) * indices.size();

        // Encapsulate staging buffer creation parameters, ensuring zero initialization
        BufferCreationInfo _stagingBufferInfo = {};
        _stagingBufferInfo.Device = m_device;
        _stagingBufferInfo.PhysicalDevice = m_physicalDevice;
        _stagingBufferInfo.Size = _bufferSize;
        _stagingBufferInfo.Usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        _stagingBufferInfo.Properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

        VkBuffer _stagingBuffer;
        VkDeviceMemory _stagingBufferMemory;
        // Create staging buffer
        VulkanBufferUtils::CreateBuffer(_stagingBufferInfo, _stagingBuffer, _stagingBufferMemory);

        // Map and copy data to the staging buffer
        void *_data = nullptr; // Ensure pointer is initialized to null
        vkMapMemory(m_device, _stagingBufferMemory, 0, _bufferSize, 0, &_data);
        memcpy(_data, indices.data(), static_cast<size_t>(_bufferSize));
        vkUnmapMemory(m_device, _stagingBufferMemory);

        // Encapsulate index buffer creation parameters, ensuring zero initialization
        BufferCreationInfo _indexBufferInfo = {};
        _indexBufferInfo.Device = m_device;
        _indexBufferInfo.PhysicalDevice = m_physicalDevice;
        _indexBufferInfo.Size = _bufferSize;
        _indexBufferInfo.Usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        _indexBufferInfo.Properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

        // Create the index buffer with device local memory
        VulkanBufferUtils::CreateBuffer(_indexBufferInfo, m_indexBuffer, m_indexBufferMemory);

        // Encapsulate buffer copying parameters, ensuring zero initialization
        BufferCopyInfo _copyInfo = {};
        _copyInfo.Device = m_device;
        _copyInfo.PhysicalDevice = m_physicalDevice;
        _copyInfo.TransferQueue = VulkanContext::GetCurrentDevice()->GetGraphicsQueue();
        _copyInfo.CommandPool = m_commandPool;
        _copyInfo.SrcBuffer = _stagingBuffer;
        _copyInfo.DstBuffer = m_indexBuffer;
        _copyInfo.Size = _bufferSize;

        // Copy data from the staging buffer to the index buffer
        VulkanBufferUtils::CopyBuffer(_copyInfo);

        // Clean up the staging buffer
        vkDestroyBuffer(m_device, _stagingBuffer, nullptr);
        vkFreeMemory(m_device, _stagingBufferMemory, nullptr);
    }


    void VulkanIndexBuffer::Bind(VkCommandBuffer commandBuffer) const
    {
        vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    }

    void VulkanIndexBuffer::Draw(VkCommandBuffer commandBuffer) const
    {
        vkCmdDrawIndexed(commandBuffer, m_indexCount, 1, 0, 0, 0);
    }

}
