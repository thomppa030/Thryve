//
// Created by kprie on 15.03.2024.
//
#pragma once

#include "SingleTimeCommandUtil.h"
#include "VkDebugUtils.h"
#include "../pch.h"

struct BufferCreationInfo {
    VkDevice device;
    VkPhysicalDevice physicalDevice;
    VkDeviceSize size;
    VkBufferUsageFlags usage;
    VkMemoryPropertyFlags properties;
};

struct BufferCopyInfo {
    VkDevice device;
    VkPhysicalDevice physicalDevice;
    VkCommandPool commandPool;
    VkQueue transferQueue;
    VkBuffer srcBuffer;
    VkBuffer dstBuffer;
    VkDeviceSize size;
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

    static void createBuffer(const BufferCreationInfo& creationInfo, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = creationInfo.size;
        bufferInfo.usage = creationInfo.usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VK_CALL(vkCreateBuffer(creationInfo.device, &bufferInfo, nullptr, &buffer));

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(creationInfo.device, buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = VulkanBufferUtils::FindMemoryType(creationInfo.physicalDevice,memRequirements.memoryTypeBits, creationInfo.properties);

        VK_CALL(vkAllocateMemory(creationInfo.device, &allocInfo, nullptr, &bufferMemory));

        VK_CALL(vkBindBufferMemory(creationInfo.device, buffer, bufferMemory, 0));
    }

    static void CopyBuffer(const BufferCopyInfo& copyInfo) {

        VkCommandBuffer commandBuffer = SingleTimeCommandUtil::BeginSingleTimeCommands(copyInfo.device, copyInfo.commandPool);

        VkBufferCopy copyRegion{};
        copyRegion.size = copyInfo.size;
        vkCmdCopyBuffer(commandBuffer, copyInfo.srcBuffer, copyInfo.dstBuffer, 1, &copyRegion);

        SingleTimeCommandUtil::EndSingleTimeCommands(copyInfo.device, copyInfo.commandPool, copyInfo.transferQueue, commandBuffer);
    }

};
