//
// Created by kprie on 15.03.2024.
//

#include "Vulkan/VulkanUniformBuffer.h"

#include "utils/VulkanBufferUtils.h"

VulkanUniformBuffer::VulkanUniformBuffer(VkDevice device, VkPhysicalDevice physicalDevice, size_t bufferSize
                                         , uint32_t bufferCount): m_Device(device), m_PhysicalDevice(physicalDevice), m_BufferSize(bufferSize), m_BufferCount(bufferCount) {
    CreateUniformBuffers();
}

VulkanUniformBuffer::~VulkanUniformBuffer() {
    for (size_t i = 0; i < m_BufferCount; i++) {
        if (m_UniformBuffersMemory[i] != VK_NULL_HANDLE) {
            vkUnmapMemory(m_Device, m_UniformBuffersMemory[i]);
        }
        vkDestroyBuffer(m_Device, m_UniformBuffers[i], nullptr);
        vkFreeMemory(m_Device, m_UniformBuffersMemory[i], nullptr);
    }
}

void VulkanUniformBuffer::UpdateUniformBuffer(uint32_t currentImage, const UniformBufferObject &uboData) {
}

void VulkanUniformBuffer::CreateUniformBuffers() {
    m_UniformBuffers.resize(m_BufferCount);
    m_UniformBuffersMemory.resize(m_BufferCount);

    for (size_t i = 0; i < m_BufferCount; i++) {
        CreateBuffer(m_BufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_UniformBuffers[i], m_UniformBuffersMemory[i]);
    }
}

void VulkanUniformBuffer::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties
    , VkBuffer&buffer, VkDeviceMemory&bufferMemory) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(m_Device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_Device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = VulkanBufferUtils::FindMemoryType(m_PhysicalDevice, memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(m_Device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(m_Device, buffer, bufferMemory, 0);
    MapMemory();
}

void VulkanUniformBuffer::MapMemory() {
    m_MappedMemory.resize(m_BufferCount);

    for (size_t i = 0; i < m_BufferCount; ++i) {
        vkMapMemory(m_Device, m_UniformBuffersMemory[i], 0, m_BufferSize, 0, &m_MappedMemory[i]);
    }
}

