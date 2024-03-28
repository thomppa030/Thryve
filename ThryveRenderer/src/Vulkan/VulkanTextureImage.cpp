//
// Created by thomppa on 3/17/24.
//

#include "Vulkan/VulkanTextureImage.h"

#include <complex>
#include "Vulkan/VulkanContext.h"
#include "stb_image.h"

#include "utils/ImageUtils.h"
#include "utils/VkDebugUtils.h"
#include "utils/VulkanBufferUtils.h"


VulkanTextureImage::VulkanTextureImage(VkCommandPool commandPool, VkCommandBuffer commandBuffer) :
    m_commandPool(commandPool), m_commandBuffer(commandBuffer)
{
    auto _deviceSelector = Thryve::Rendering::VulkanContext::Get()->GetDevice();
    m_device = _deviceSelector->GetLogicalDevice();
    m_PhysicalDevice = _deviceSelector->GetPhysicalDevice();
    m_transferQueue = _deviceSelector->GetGraphicsQueue();
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

void VulkanTextureImage::createTextureImage(const std::string &fileName)
{
    int texWidth, texHeight, texChannels;
    const stbi_uc *pixels = stbi_load(fileName.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    const VkDeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels)
    {
        throw std::runtime_error("failed to load texture image!");
    }

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    BufferCreationInfo _stagingBufferCreateInfo{};
    _stagingBufferCreateInfo.Device = m_device;
    _stagingBufferCreateInfo.PhysicalDevice = m_PhysicalDevice;
    _stagingBufferCreateInfo.Size = imageSize;
    _stagingBufferCreateInfo.Usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    _stagingBufferCreateInfo.Properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    VulkanBufferUtils::CreateBuffer(_stagingBufferCreateInfo, stagingBuffer, stagingBufferMemory);

    void *data;
    vkMapMemory(m_device, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(m_device, stagingBufferMemory);

    stbi_image_free((void *)(pixels));

    ImageUtils::CreateImage(static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), VK_FORMAT_R8G8B8A8_SRGB,
                            VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_textureImage, m_textureImageMemory);

    ImageUtils::transitionImageLayout(m_device, m_commandPool, m_transferQueue, m_textureImage, VK_FORMAT_B8G8R8A8_SRGB,
                                      VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    ImageUtils::copyBufferToImage(m_device, m_commandPool, m_transferQueue, stagingBuffer, m_textureImage, texWidth,
                                  texHeight);
    ImageUtils::transitionImageLayout(m_device, m_commandPool, m_transferQueue, m_textureImage, VK_FORMAT_B8G8R8A8_SRGB,
                                      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(m_device, stagingBuffer, nullptr);
    vkFreeMemory(m_device, stagingBufferMemory, nullptr);
}

void VulkanTextureImage::createTextureImageView()
{
    ImageUtils::CreateImageView(m_textureImage, m_textureImageView, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
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
