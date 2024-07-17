//
// Created by kprie on 15.07.2024.
//
#pragma once

#include "vk_mem_alloc.h"

#define MOVABLE_ONLY(CLASS_NAME)                   \
CLASS_NAME(const CLASS_NAME&) = delete;            \
CLASS_NAME& operator=(const CLASS_NAME&) = delete; \
CLASS_NAME(CLASS_NAME&&) noexcept = default;       \
CLASS_NAME& operator=(CLASS_NAME&&) noexcept = default;

namespace Thryve::Rendering {
    class VulkanBuffer final {
    public:
        MOVABLE_ONLY(VulkanBuffer);

        explicit VulkanBuffer(VmaAllocator vmaAllocator, VkDeviceSize size, VkBufferUsageFlags usage,
                              VulkanBuffer* actualBuffer, const std::string& name = "");

        explicit VulkanBuffer(VmaAllocator vmaAllocator, const VkBufferCreateInfo& createInfo,
                              const VmaAllocationCreateInfo& allocInfo, const std::string& name = "");

        ~VulkanBuffer();

        [[nodiscard]] VkDeviceSize GetSize() const;

        void Upload(VkDeviceSize offset = 0) const;

        void Upload(VkDeviceSize offset, VkDeviceSize size) const;

        void UploadStagingBufferToGPU(const VkCommandBuffer& commandBuffer, uint64_t srcOffset = 0, uint64_t dstOffset = 0) const;

        void CopyDataToBuffer(const void* data, size_t size) const;

        [[nodiscard]] VkBuffer GetBuffer() const {return m_Buffer; }

        [[nodiscard]] VkDeviceAddress GetDeviceAddress() const;

        VkBufferView RequestBufferView(VkFormat viewFormat);

    private:
        VmaAllocator m_Allocator;
        VkDeviceSize m_Size;
        VkBufferUsageFlags m_Usage;
        VmaAllocationCreateInfo m_AllocCreateInfo;
        VkBuffer m_Buffer = VK_NULL_HANDLE;
        VulkanBuffer* m_ActualBufferIfStaging = nullptr;
        VmaAllocation m_Allocation = nullptr;
        VmaAllocationInfo m_AllocationInfo = {};
        mutable VkDeviceAddress m_BufferDeviceAddress = 0;
        mutable void* m_MappedMemory = nullptr;
        std::unordered_map<VkFormat, VkBufferView> m_BufferViews;
    };
} // namespace Thryve::Rendering