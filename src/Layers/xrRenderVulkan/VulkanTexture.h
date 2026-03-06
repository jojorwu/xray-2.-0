#pragma once

#include "VulkanHW.h"

class CVulkanTexture
{
    u32 m_ref_count;
public:
    CVulkanTexture();
    ~CVulkanTexture();

    void Create(uint32_t width, uint32_t height, uint32_t mips, VkFormat format, VkImageUsageFlags usage);
    void Destroy();

    void Load(LPCSTR name, uint32_t& size);
    void LoadFromMemory(void* data, uint32_t size, uint32_t& out_size);
    void UploadData(void* data, uint32_t size);

    VkImage GetImage() { return m_image; }
    VkImageView GetImageView() { return m_image_view; }

    uint32_t GetWidth() const { return m_width; }
    uint32_t GetHeight() const { return m_height; }

    // Compatibility methods for ID3DBaseTexture
    u32 GetType() { return 3; } // D3DRTYPE_TEXTURE (3 in DX9)
    void AddRef() { m_ref_count++; }
    void Release()
    {
        m_ref_count--;
        if (m_ref_count == 0)
            xr_delete(this);
    }

private:
    VkImage m_image;
    VmaAllocation m_allocation;
    VkImageView m_image_view;
    VkFormat m_format;
    uint32_t m_width, m_height;

    void CreateImageView();
};
