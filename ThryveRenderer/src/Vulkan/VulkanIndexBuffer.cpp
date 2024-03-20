//
// Created by kprie on 15.03.2024.
//

#include "Vulkan/VulkanIndexBuffer.h"
#include "utils/VulkanBufferUtils.h"

namespace Thryve::Rendering {
    VulkanIndexBuffer::VulkanIndexBuffer(const VkDevice device, const VkPhysicalDevice physicalDevice
                                         , const VkCommandPool commandPool, const VkQueue graphicsQueue) :
        m_device(device),
        m_physicalDevice(physicalDevice),
        m_commandPool(commandPool),
        m_graphicsQueue(graphicsQueue)
    {
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
        _stagingBufferInfo.device = m_device;
        _stagingBufferInfo.physicalDevice = m_physicalDevice;
        _stagingBufferInfo.size = _bufferSize;
        _stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        _stagingBufferInfo.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

        VkBuffer _stagingBuffer;
        VkDeviceMemory _stagingBufferMemory;
        // Create staging buffer
        VulkanBufferUtils::createBuffer(_stagingBufferInfo, _stagingBuffer, _stagingBufferMemory);

        // Map and copy data to the staging buffer
        void *_data = nullptr; // Ensure pointer is initialized to null
        vkMapMemory(m_device, _stagingBufferMemory, 0, _bufferSize, 0, &_data);
        memcpy(_data, indices.data(), static_cast<size_t>(_bufferSize));
        vkUnmapMemory(m_device, _stagingBufferMemory);

        // Encapsulate index buffer creation parameters, ensuring zero initialization
        BufferCreationInfo _indexBufferInfo = {};
        _indexBufferInfo.device = m_device;
        _indexBufferInfo.physicalDevice = m_physicalDevice;
        _indexBufferInfo.size = _bufferSize;
        _indexBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        _indexBufferInfo.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

        // Create the index buffer with device local memory
        VulkanBufferUtils::createBuffer(_indexBufferInfo, m_indexBuffer, m_indexBufferMemory);

        // Encapsulate buffer copying parameters, ensuring zero initialization
        BufferCopyInfo copyInfo = {};
        copyInfo.device = m_device;
        copyInfo.physicalDevice = m_physicalDevice;
        copyInfo.commandPool = m_commandPool;
        copyInfo.transferQueue = m_graphicsQueue;
        copyInfo.srcBuffer = _stagingBuffer;
        copyInfo.dstBuffer = m_indexBuffer;
        copyInfo.size = _bufferSize;

        // Copy data from the staging buffer to the index buffer
        VulkanBufferUtils::CopyBuffer(copyInfo);

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
