//
// Created by thomppa on 3/17/24.
//

#include "../../include/Vulkan/VulkanTextureImage.h"

#include <complex>
#include <../../../cmake-build-debug/generated/Config.h>
#include <../../external/stb/stb_image.h>

#include "../../include/utils/ImageUtils.h"
#include "../../include/utils/VulkanBufferUtils.h"


VulkanTextureImage::VulkanTextureImage(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool
                                       , VkQueue graphicsQueue, VkCommandBuffer commandBuffer): m_device(device), m_PhysicalDevice(physicalDevice),
                                                             m_commandPool(commandPool),
                                                             m_transferQueue(graphicsQueue),
                                                             m_commandBuffer(commandBuffer) {
}

VulkanTextureImage::~VulkanTextureImage() {
    cleanup();
}


void VulkanTextureImage::cleanup() {
    vkDestroySampler(m_device, m_TextureSampler, nullptr);
    vkDestroyImageView(m_device, m_textureImageView, nullptr);
    vkDestroyImage(m_device, m_textureImage, nullptr);
    vkFreeMemory(m_device, m_textureImageMemory, nullptr);
}

void VulkanTextureImage::createTextureImage(const std::string &fileName) {
    int texWidth, texHeight, texChannels;
    const stbi_uc* pixels = stbi_load(fileName.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    const VkDeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    BufferCreationInfo stagingBufferCreateInfo {};
    stagingBufferCreateInfo.device = m_device;
    stagingBufferCreateInfo.physicalDevice = m_PhysicalDevice;
    stagingBufferCreateInfo.size = imageSize;
    stagingBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    stagingBufferCreateInfo.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    VulkanBufferUtils::createBuffer(stagingBufferCreateInfo, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(m_device, stagingBufferMemory, 0, imageSize,0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(m_device, stagingBufferMemory);

    stbi_image_free((void*)(pixels));

    createImage(static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_textureImage, m_textureImageMemory);

    ImageUtils::transitionImageLayout(m_device, m_commandPool, m_transferQueue, m_textureImage
                                      , VK_FORMAT_B8G8R8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED
                                      , VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    ImageUtils::copyBufferToImage(m_device, m_commandPool, m_transferQueue, stagingBuffer, m_textureImage, texWidth, texHeight);
    ImageUtils::transitionImageLayout(m_device, m_commandPool, m_transferQueue, m_textureImage
                                      , VK_FORMAT_B8G8R8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(m_device, stagingBuffer, nullptr);
    vkFreeMemory(m_device, stagingBufferMemory, nullptr);
}

void VulkanTextureImage::createTextureImageView() {
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = m_textureImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VK_CALL(vkCreateImageView(m_device, &viewInfo, nullptr, &m_textureImageView));
}

void VulkanTextureImage::createTextureSampler() {
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;

    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(m_PhysicalDevice, &properties);

    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;

    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    VK_CALL(vkCreateSampler(m_device, &samplerInfo, nullptr, &m_TextureSampler));
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

    VK_CALL(vkCreateImage(m_device, &imageCreateInfo, nullptr, &image));

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(m_device, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = VulkanBufferUtils::FindMemoryType(m_PhysicalDevice,memRequirements.memoryTypeBits, properties);

    VK_CALL(vkAllocateMemory(m_device, &allocInfo, nullptr, &imageMemory));

    vkBindImageMemory(m_device, image, imageMemory, 0);
}