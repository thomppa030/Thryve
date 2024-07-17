//
// Created by kprie on 17.04.2024.
//
#pragma once
#include "../imGui/imGuiLayer.h"

namespace Thryve::UI {

    class VulkanDescriptorPool {
    public:
        VulkanDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo& poolInfo) :
            m_device{device}, m_descriptorPool{VK_NULL_HANDLE}
        {
            if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to create descriptor pool!");
            }
        }

        ~VulkanDescriptorPool()
        {
            if (m_descriptorPool != VK_NULL_HANDLE)
            {
                vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);
            }
        }

        [[nodiscard]] VkDescriptorPool Get() const { return m_descriptorPool; }

    private:
        VkDevice m_device;
        VkDescriptorPool m_descriptorPool;
    };

    class VulkanCommandBuffer {
    public:
        VulkanCommandBuffer(const VkDevice device, const VkCommandPool commandPool) :
            device_(device), commandPool_(commandPool), commandBuffer_(VK_NULL_HANDLE)
        {
            VkCommandBufferAllocateInfo _allocInfo{};
            _allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            _allocInfo.commandPool = commandPool_;
            _allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            _allocInfo.commandBufferCount = 1;

            if (vkAllocateCommandBuffers(device_, &_allocInfo, &commandBuffer_) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to allocate command buffer!");
            }
        }

        ~VulkanCommandBuffer()
        {
            if (commandBuffer_ != VK_NULL_HANDLE)
            {
                vkFreeCommandBuffers(device_, commandPool_, 1, &commandBuffer_);
            }
        }

        [[nodiscard]] VkCommandBuffer Get() const { return commandBuffer_; }

    private:
        VkDevice device_;
        VkCommandPool commandPool_;
        VkCommandBuffer commandBuffer_;
    };

    class VulkanImGuiLayer final : public ImGuiLayer {

    public:
        // TODO Implement and utilize!
        ~VulkanImGuiLayer() override;
        void OnAttach() override;
        void OnDetach() override;
        void OnImGuiRender() override;

        void Begin() override;
        void End() override;

    private:
        std::unique_ptr<VulkanDescriptorPool> m_imguiPool;
    };
} // namespace Thryve::UI
// Thryve
