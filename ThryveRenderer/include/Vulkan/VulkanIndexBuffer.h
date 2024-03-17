//
// Created by kprie on 15.03.2024.
//
#pragma once


class VulkanIndexBuffer {
public:
    VulkanIndexBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool
                      , VkQueue graphicsQueue);

    ~VulkanIndexBuffer();

    void Create(const std::vector<uint32_t> &indices);

    void Bind(VkCommandBuffer commandBuffer) const;
    void Draw(VkCommandBuffer commandBuffer) const;

    [[nodiscard]] VkBuffer GetIndexBuffer() const {return m_indexBuffer;}
    [[nodiscard]] VkDeviceMemory GetIndexBufferMemory() const {return m_indexBufferMemory;}
    [[nodiscard]] uint32_t GetIndexCount() const {return m_indexCount;}

    // Optionally, add methods to update or modify the index buffer
private:
    VkDevice m_device;
    VkPhysicalDevice m_physicalDevice;
    VkCommandPool m_commandPool;
    VkQueue m_graphicsQueue;
    VkBuffer m_indexBuffer;
    VkDeviceMemory m_indexBufferMemory;
    uint32_t m_indexCount = 0;
};
