//
// Created by thomppa on 3/17/24.
//

#include "VulkanTextureImage.h"

#include <complex>
#include <Config.h>
#include <stb_image.h>

#include "utils/VulkanBufferUtils.h"

VulkanTextureImage::VulkanTextureImage(VulkanDeviceSelector* DeviceSelector): m_deviceSelector(DeviceSelector) {
}

void VulkanTextureImage::cleanup() {
}

void VulkanTextureImage::createTextureImage(const std::string &fileName) {
    int texWidth, texHeight, texChannels;
    const stbi_uc* pixels = stbi_load((std::string(RESOURCE_DIR)+"/statue.jpg").c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    BufferCreationInfo stagingBufferCreateInfo {};
    stagingBufferCreateInfo.device = m_deviceSelector->GetLogicalDevice();
    stagingBufferCreateInfo.physicalDevice = m_deviceSelector->GetPhysicalDevice();
    stagingBufferCreateInfo.size = imageSize;
    stagingBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    stagingBufferCreateInfo.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    VulkanBufferUtils::createBuffer(stagingBufferCreateInfo, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(m_deviceSelector->GetLogicalDevice(), stagingBufferMemory, 0, imageSize,0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(m_deviceSelector->GetLogicalDevice(), stagingBufferMemory);

    stbi_image_free((void*)(pixels));

    createImage(static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_textureImage, m_textureImageMemory);
}

void VulkanTextureImage::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
    VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage&image, VkDeviceMemory&imageMemory) const {

    VkImageCreateInfo imageCreateInfo {};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.extent.width = width;
    imageCreateInfo.extent.height = height;
    imageCreateInfo.extent.depth = 1;
    imageCreateInfo.mipLevels = 1;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.tiling = tiling;
    imageCreateInfo.format = format;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCreateInfo.usage = usage;
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.flags = 0;

    VK_CALL(vkCreateImage(m_deviceSelector->GetLogicalDevice(), &imageCreateInfo, nullptr, &image));

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(m_deviceSelector->GetLogicalDevice(), image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = VulkanBufferUtils::FindMemoryType(m_deviceSelector->GetPhysicalDevice(),memRequirements.memoryTypeBits, properties);

    VK_CALL(vkAllocateMemory(m_deviceSelector->GetLogicalDevice(), &allocInfo, nullptr, &imageMemory));

    vkBindImageMemory(m_deviceSelector->GetLogicalDevice(), image, imageMemory, 0);
}

void VulkanTextureImage::createSampler() {
}

void VulkanTextureImage::allocate() {
}

void VulkanTextureImage::copyBufferToImage(VkBuffer buffer, uint32_t width, uint32_t height) {
}
