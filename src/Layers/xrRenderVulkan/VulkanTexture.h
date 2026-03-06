#pragma once

#include "VulkanHW.h"

class CVulkanTexture
{
public:
    CVulkanTexture();
    ~CVulkanTexture();

    void Create(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage);
    void Destroy();

    void Load(LPCSTR name);

    VkImage GetImage() { return m_image; }
    VkImageView GetImageView() { return m_image_view; }

private:
    VkImage m_image;
    VmaAllocation m_allocation;
    VkImageView m_image_view;
    VkFormat m_format;
    uint32_t m_width, m_height;

    void CreateImageView();
};
