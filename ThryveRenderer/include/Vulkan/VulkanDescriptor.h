//
// Created by kprie on 15.03.2024.
//

#ifndef VULKANDESCRIPTOR_H
#define VULKANDESCRIPTOR_H
#include "pch.h"


class VulkanDescriptor {
    public:
        // Constructor
        VulkanDescriptor(VkDescriptorType type, uint32_t binding, VkShaderStageFlags stageFlags)
        : m_type(type), m_binding(binding), m_stageFlags(stageFlags) {}

        // Accessors
        [[nodiscard]] VkDescriptorType GetType() const { return m_type; }
        [[nodiscard]] uint32_t GetBinding() const { return m_binding; }
        [[nodiscard]] VkShaderStageFlags GetStageFlags() const { return m_stageFlags; }

    private:
        VkDescriptorType m_type;
        uint32_t m_binding;
        VkShaderStageFlags m_stageFlags;
        // Additional details like buffer or image view references can be added here
};



#endif //VULKANDESCRIPTOR_H
