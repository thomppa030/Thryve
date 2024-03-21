//
// Created by kprie on 14.03.2024.
//
#pragma once

#include "pch.h"

#include "Core/Ref.h"
#include "Vertex2D.h"
#include "utils/VkDebugUtils.h"
#include "utils/VulkanBufferUtils.h"

namespace Thryve::Rendering {
    template <typename VertexType>
    class VulkanVertexBuffer {
    public:
        VulkanVertexBuffer(const VkDevice _device, VkPhysicalDevice physicalDevice, const VkCommandPool commandPool, const VkQueue graphicsQueue) :
            m_commandPool(commandPool), m_graphicsQueue(graphicsQueue), m_device{_device},
            m_physicalDevice{physicalDevice}
        {
        }

        ~VulkanVertexBuffer() { Cleanup(); }

        // Disallow copying to avoid accidental duplications
        VulkanVertexBuffer(const VulkanVertexBuffer &) = delete;
        VulkanVertexBuffer &operator=(const VulkanVertexBuffer &) = delete;

        // Allow move semantics for efficient resource management
        VulkanVertexBuffer(VulkanVertexBuffer &&other) noexcept :
            m_device(other.m_device), m_physicalDevice(other.m_physicalDevice), m_commandPool(other.m_commandPool),
            m_graphicsQueue(other.m_graphicsQueue)
        {
            m_buffer = other.m_buffer;
            m_bufferMemory = other.m_bufferMemory;
            m_vertexCount = other.m_vertexCount;

            other.m_buffer = VK_NULL_HANDLE;
            other.m_bufferMemory = VK_NULL_HANDLE;
            other.m_vertexCount = 0;
        }

        /**
         * Assigns the contents of the given VulkanVertexBuffer to this VulkanVertexBuffer using move semantics.
         *
         * @param other The VulkanVertexBuffer to move from.
         * @return A reference to this VulkanVertexBuffer.
         */
        VulkanVertexBuffer<VertexType> &operator=(VulkanVertexBuffer &&other) noexcept
        {
            if (this != &other)
            {
                Cleanup();
                // Transfer ownership
                m_device = other.m_device;
                m_physicalDevice = other.m_physicalDevice;
                m_commandPool = other.m_commandPool;
                m_graphicsQueue = other.m_graphicsQueue;
                m_buffer = other.m_buffer;
                m_bufferMemory = other.m_bufferMemory;
                m_vertexCount = other.m_vertexCount;

                // Reset other
                other.m_buffer = VK_NULL_HANDLE;
                other.m_bufferMemory = VK_NULL_HANDLE;
                other.m_vertexCount = 0;
            }
            return *this;
        }


        void Bind(VkCommandBuffer commandBuffer) const;
        void Draw(VkCommandBuffer commandBuffer) const;

        /**
         * Create a vertex buffer from the given vertices.
         *
         * @param vertices The vector of vertices to create the buffer from.
         */
        void Create(const std::vector<VertexType> &vertices)
        {
            m_vertexCount = vertices.size();
            const VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

            VkBuffer stagingBuffer;
            VkDeviceMemory stagingBufferMemory;

            BufferCreationInfo stagingBufferInfo = {};
            stagingBufferInfo.Device = m_device;
            stagingBufferInfo.PhysicalDevice = m_physicalDevice;
            stagingBufferInfo.Size = bufferSize;
            stagingBufferInfo.Usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            stagingBufferInfo.Properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

            // Create staging buffer
            VulkanBufferUtils::CreateBuffer(stagingBufferInfo, stagingBuffer, stagingBufferMemory);

            // Map memory and copy vertex data
            void *data = nullptr; // Initialize to nullptr for safety
            VK_CALL(vkMapMemory(m_device, stagingBufferMemory, 0, bufferSize, 0, &data));
            memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
            vkUnmapMemory(m_device, stagingBufferMemory);

            BufferCreationInfo vertexBufferInfo = {};
            vertexBufferInfo.Device = m_device;
            vertexBufferInfo.PhysicalDevice = m_physicalDevice;
            vertexBufferInfo.Size = bufferSize;
            vertexBufferInfo.Usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            vertexBufferInfo.Properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

            // Create vertex buffer
            VulkanBufferUtils::CreateBuffer(vertexBufferInfo, m_buffer, m_bufferMemory);

            BufferCopyInfo copyInfo = {};
            copyInfo.Device = m_device;
            copyInfo.PhysicalDevice = m_physicalDevice;
            copyInfo.TransferQueue = m_graphicsQueue;
            copyInfo.CommandPool = m_commandPool;
            copyInfo.SrcBuffer = stagingBuffer;
            copyInfo.DstBuffer = m_buffer;
            copyInfo.Size = bufferSize;

            // Copy vertex data from the staging buffer to the vertex buffer
            VulkanBufferUtils::CopyBuffer(copyInfo);

            // Clean up staging buffer
            vkDestroyBuffer(m_device, stagingBuffer, nullptr);
            vkFreeMemory(m_device, stagingBufferMemory, nullptr);
        }


        void Cleanup()
        {
            if (m_buffer != VK_NULL_HANDLE)
            {
                vkDestroyBuffer(m_device, m_buffer, nullptr);
                m_buffer = VK_NULL_HANDLE;
            }
            if (m_bufferMemory != VK_NULL_HANDLE)
            {
                vkFreeMemory(m_device, m_bufferMemory, nullptr);
            }
        }

        [[nodiscard]] VkBuffer GetBuffer() const { return m_buffer; }
        [[nodiscard]] VkDeviceMemory GetBufferMemory() const { return m_bufferMemory; }
        [[nodiscard]] size_t GetVertexCount() const { return m_vertexCount; }
        [[nodiscard]] VkDeviceSize GetSize() const { return sizeof(VertexType) * m_vertexCount; }

    private:
        VkDevice m_device;
        VkPhysicalDevice m_physicalDevice;
        VkCommandPool m_commandPool;
        VkQueue m_graphicsQueue;

        VkBuffer m_buffer = VK_NULL_HANDLE;
        VkDeviceMemory m_bufferMemory = VK_NULL_HANDLE;
        size_t m_vertexCount = 0;
    };

    template <typename VertexType>
    void VulkanVertexBuffer<VertexType>::Bind(VkCommandBuffer commandBuffer) const
    {
        const VkBuffer vertexBuffers[] = {m_buffer};
        constexpr VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    }

    template <typename VertexType>
    void VulkanVertexBuffer<VertexType>::Draw(VkCommandBuffer commandBuffer) const
    {
        vkCmdDraw(commandBuffer, m_vertexCount, 1, 0, 0);
    }

    template class VulkanVertexBuffer<Vertex2D>;
    template class VulkanVertexBuffer<Vertex3D>;
} // namespace Thryve::Rendering
