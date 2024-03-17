//
// Created by thomppa on 3/17/24.
//
#pragma once
#include "pch.h"
#include "VulkanDeviceSelector.h"


class VulkanTextureImage {
public:
    VulkanTextureImage(VulkanDeviceSelector* DeviceSelector);
    ~VulkanTextureImage();

    //Accessors
private:
    VulkanDeviceSelector* m_deviceSelector;
    VkImage m_textureImage{};
    VkDeviceMemory m_textureImageMemory{};
    VkImageView imageView{};
    VkSampler sampler{};

    void cleanup();
    void createTextureImage(const std::string& fileName);
    void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) const;
    void createSampler();
    void allocate();
};
