//
// Created by thomppa on 3/17/24.
//
#pragma once
#include "pch.h"
#include "VulkanDeviceSelector.h"


class VulkanTextureImage {
public:
    VulkanTextureImage(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, VkCommandBuffer commandBuffer);
    ~VulkanTextureImage();

    //Accessors
    void createTextureImage(const std::string& fileName);
    void createTextureImageView();
    void createTextureSampler();
    [[nodiscard]] VkImage GetTextureImage() const {return m_textureImage;}
    [[nodiscard]] VkImageView GetTextureImageView() const {return m_textureImageView;}
    [[nodiscard]] VkSampler GetTextureSampler() const {return m_TextureSampler;}

private:
    VkDevice m_device;
    VkPhysicalDevice m_PhysicalDevice;
    VkCommandPool m_commandPool;
    VkQueue m_transferQueue;
    VkCommandBuffer m_commandBuffer;

    VkImage m_textureImage{};
    VkImageView m_textureImageView{};
    VkSampler m_TextureSampler{};
    VkDeviceMemory m_textureImageMemory{};
    VkImageView imageView{};
    VkSampler sampler{};

    void cleanup();
    void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) const;
};
