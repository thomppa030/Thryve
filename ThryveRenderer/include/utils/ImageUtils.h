//
// Created by thomppa on 3/17/24.
//
#pragma once

#include "pch.h"
#include "SingleTimeCommandUtil.h"

static void transitionImageLayout(const VkDevice device, const VkCommandPool commandPool, VkQueue transferQueue, VkCommandBuffer commandBuffer, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
    SingleTimeCommandUtil::BeginSingleTimeCommands(device, commandPool);

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;

    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    barrier.srcAccessMask = 0; // TODO
    barrier.dstAccessMask = 0; // TODO

    vkCmdPipelineBarrier(
        commandBuffer,
        0 /* TODO */, 0 /* TODO */,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    SingleTimeCommandUtil::EndSingleTimeCommands(device, commandPool,transferQueue, commandBuffer);
};

static void copyBufferToImage(VkDevice device, VkCommandBuffer commandBuffer, VkCommandPool commandPool,
                              VkQueue copyQueue, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
    SingleTimeCommandUtil::BeginSingleTimeCommands(device, commandPool);

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = {
        width,
        height,
        1
    };

    vkCmdCopyBufferToImage(
        commandBuffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );

    SingleTimeCommandUtil::EndSingleTimeCommands(device, commandPool, copyQueue, commandBuffer);
};

