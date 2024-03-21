//
// Created by kprie on 15.03.2024.
//
#pragma once

#include "SingleTimeCommandUtil.h"
#include "VkDebugUtils.h"
#include "../pch.h"

struct BufferCreationInfo {
    VkDevice Device;
    VkPhysicalDevice PhysicalDevice;
    VkDeviceSize Size;
    VkBufferUsageFlags Usage;
    VkMemoryPropertyFlags Properties;
};

struct BufferCopyInfo {
    VkDevice Device;
    VkPhysicalDevice PhysicalDevice;
    VkCommandPool CommandPool;
    VkQueue TransferQueue;
    VkBuffer SrcBuffer;
    VkBuffer DstBuffer;
    VkDeviceSize Size;
};

class VulkanBufferUtils {
public:

    static uint32_t FindMemoryType(const VkPhysicalDevice physicalDevice, const uint32_t typeFilter, const VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        throw std::runtime_error("failed to find suitable memory type!");
    }

    static void CreateBuffer(const BufferCreationInfo& creationInfo, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = creationInfo.Size;
        bufferInfo.usage = creationInfo.Usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VK_CALL(vkCreateBuffer(creationInfo.Device, &bufferInfo, nullptr, &buffer));

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(creationInfo.Device, buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = VulkanBufferUtils::FindMemoryType(creationInfo.PhysicalDevice,memRequirements.memoryTypeBits, creationInfo.Properties);

        VK_CALL(vkAllocateMemory(creationInfo.Device, &allocInfo, nullptr, &bufferMemory));

        VK_CALL(vkBindBufferMemory(creationInfo.Device, buffer, bufferMemory, 0));
    }

    static void CopyBuffer(const BufferCopyInfo& copyInfo) {
        const VkCommandBuffer commandBuffer = SingleTimeCommandUtil::BeginSingleTimeCommands(copyInfo.Device, copyInfo.CommandPool);

        VkBufferCopy copyRegion{};
        copyRegion.size = copyInfo.Size;
        vkCmdCopyBuffer(commandBuffer, copyInfo.SrcBuffer, copyInfo.DstBuffer, 1, &copyRegion);

        SingleTimeCommandUtil::EndSingleTimeCommands(copyInfo.Device, copyInfo.CommandPool, copyInfo.TransferQueue, commandBuffer);
    }

};
