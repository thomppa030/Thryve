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
    [[nodiscard]] VkImage GetTextureImage() const {return m_textureImage;}

private:
    VkDevice m_device;
    VkPhysicalDevice m_PhysicalDevice;
    VkCommandPool m_commandPool;
    VkQueue m_transferQueue;
    VkCommandBuffer m_commandBuffer;

    VkImage m_textureImage{};
    VkDeviceMemory m_textureImageMemory{};
    VkImageView imageView{};
    VkSampler sampler{};

    void cleanup();
    void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) const;
    void createSampler();
    void allocate();
};
