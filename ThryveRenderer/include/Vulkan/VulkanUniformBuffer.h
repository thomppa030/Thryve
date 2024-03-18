//
// Created by kprie on 15.03.2024.
//
#pragma once

#include "UniformBufferObject.h"
#include "glm/glm.hpp"


class VulkanUniformBuffer {
public:
    VulkanUniformBuffer(VkDevice device, VkPhysicalDevice physicalDevice, size_t bufferSize, uint32_t bufferCount);
    ~VulkanUniformBuffer();

    void UpdateUniformBuffer(uint32_t currentImage, const UniformBufferObject& uboData);
    [[nodiscard]] VkBuffer GetBuffer(const uint32_t index) const {return m_UniformBuffers[index];}
    [[nodiscard]] void* GetMappedMemory(const uint32_t index) const { return m_MappedMemory[index]; }

private:
    VkDevice m_Device;
    VkPhysicalDevice m_PhysicalDevice;
    std::vector<VkBuffer> m_UniformBuffers;
    std::vector<VkDeviceMemory> m_UniformBuffersMemory;
    size_t m_BufferSize;
    uint32_t m_BufferCount;
    std::vector<void*> m_MappedMemory;

    void CreateUniformBuffers();
    void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    void MapMemory();
};