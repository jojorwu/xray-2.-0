#include "stdafx.h"
#include "VulkanTexture.h"

CVulkanTexture::CVulkanTexture()
{
    m_ref_count = 0;
    m_image = VK_NULL_HANDLE;
    m_allocation = VK_NULL_HANDLE;
    m_image_view = VK_NULL_HANDLE;
    m_sampler = VK_NULL_HANDLE;
    m_format = VK_FORMAT_UNDEFINED;
    m_width = 0;
    m_height = 0;
    m_mips = 0;
}

CVulkanTexture::~CVulkanTexture()
{
    Destroy();
}

void CVulkanTexture::Create(uint32_t width, uint32_t height, uint32_t mips, VkFormat format, VkImageUsageFlags usage)
{
    m_width = width;
    m_height = height;
    m_mips = mips;
    m_format = format;

    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = mips;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    if (vmaCreateImage(VulkanHW.GetAllocator(), &imageInfo, &allocInfo, &m_image, &m_allocation, nullptr) != VK_SUCCESS)
    {
        Msg("! Vulkan: Failed to create image!");
    }

    CreateImageView();
    VulkanHW.CreateSampler(VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, m_sampler);
}

void CVulkanTexture::Destroy()
{
    if (m_sampler != VK_NULL_HANDLE)
    {
        vkDestroySampler(VulkanHW.GetDevice(), m_sampler, nullptr);
        m_sampler = VK_NULL_HANDLE;
    }

    if (m_image_view != VK_NULL_HANDLE)
    {
        vkDestroyImageView(VulkanHW.GetDevice(), m_image_view, nullptr);
        m_image_view = VK_NULL_HANDLE;
    }

    if (m_image != VK_NULL_HANDLE)
    {
        vmaDestroyImage(VulkanHW.GetAllocator(), m_image, m_allocation);
        m_image = VK_NULL_HANDLE;
        m_allocation = VK_NULL_HANDLE;
    }
}

void CVulkanTexture::Load(LPCSTR name, uint32_t& out_size)
{
    Msg("Vulkan: Loading texture %s", name);
    out_size = 0;

    string_path fn;
    if (!FS.exist(fn, "$game_textures$", name, ".dds"))
    {
        if (!FS.exist(fn, "$level$", name, ".dds"))
        {
            if (!FS.exist(fn, "$game_saves$", name, ".dds"))
            {
                Msg("! Vulkan: Can't find texture %s", name);
                return;
            }
        }
    }

    IReader* r = FS.r_open(fn);
    if (!r) return;

    LoadFromMemory(r->pointer(), r->length(), out_size);

    FS.r_close(r);
}

struct DDS_PIXELFORMAT
{
    uint32_t dwSize;
    uint32_t dwFlags;
    uint32_t dwFourCC;
    uint32_t dwRGBBitCount;
    uint32_t dwRBitMask;
    uint32_t dwGBitMask;
    uint32_t dwBBitMask;
    uint32_t dwABitMask;
};

struct DDS_HEADER
{
    uint32_t dwSize;
    uint32_t dwFlags;
    uint32_t dwHeight;
    uint32_t dwWidth;
    uint32_t dwPitchOrLinearSize;
    uint32_t dwDepth;
    uint32_t dwMipMapCount;
    uint32_t dwReserved1[11];
    DDS_PIXELFORMAT ddspf;
    uint32_t dwCaps;
    uint32_t dwCaps2;
    uint32_t dwCaps3;
    uint32_t dwCaps4;
    uint32_t dwReserved2;
};

#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
                ((uint32_t)(uint8_t)(ch0) | ((uint32_t)(uint8_t)(ch1) << 8) |       \
                ((uint32_t)(uint8_t)(ch2) << 16) | ((uint32_t)(uint8_t)(ch3) << 24 ))
#endif

void CVulkanTexture::LoadFromMemory(void* data, uint32_t size, uint32_t& out_size)
{
    out_size = 0;
    uint32_t* magic = (uint32_t*)data;
    if (*magic != 0x20534444) // "DDS "
    {
        Msg("! Vulkan: Invalid DDS magic");
        return;
    }

    DDS_HEADER* header = (DDS_HEADER*)((uint8_t*)data + 4);
    m_width = header->dwWidth;
    m_height = header->dwHeight;
    uint32_t mips = header->dwMipMapCount ? header->dwMipMapCount : 1;

    VkFormat format = VK_FORMAT_UNDEFINED;
    bool compressed = false;
    uint32_t block_size = 0;

    if (header->ddspf.dwFlags & 0x4) // DDPF_FOURCC
    {
        switch (header->ddspf.dwFourCC)
        {
        case MAKEFOURCC('D', 'X', 'T', '1'):
            format = VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
            compressed = true;
            block_size = 8;
            break;
        case MAKEFOURCC('D', 'X', 'T', '3'):
            format = VK_FORMAT_BC2_UNORM_BLOCK;
            compressed = true;
            block_size = 16;
            break;
        case MAKEFOURCC('D', 'X', 'T', '5'):
            format = VK_FORMAT_BC3_UNORM_BLOCK;
            compressed = true;
            block_size = 16;
            break;
        }
    }
    else if (header->ddspf.dwFlags & 0x40) // DDPF_RGB
    {
        if (header->ddspf.dwRGBBitCount == 32)
            format = VK_FORMAT_B8G8R8A8_UNORM;
    }

    if (format == VK_FORMAT_UNDEFINED)
    {
        Msg("! Vulkan: Unsupported DDS format");
        return;
    }

    Create(m_width, m_height, mips, format, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

    uint8_t* pixels = (uint8_t*)data + 4 + sizeof(DDS_HEADER);

    VkBuffer stagingBuffer;
    VmaAllocation stagingAllocation;
    uint32_t dataSize = size - (pixels - (uint8_t*)data);
    out_size = dataSize;
    VulkanHW.CreateBuffer(dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, stagingBuffer, stagingAllocation, VMA_ALLOCATION_CREATE_MAPPED_BIT);

    void* mappedData;
    vmaMapMemory(VulkanHW.GetAllocator(), stagingAllocation, &mappedData);
    memcpy(mappedData, pixels, dataSize);
    vmaUnmapMemory(VulkanHW.GetAllocator(), stagingAllocation);

    xr_vector<VkBufferImageCopy> regions;
    uint32_t offset = 0;
    uint32_t w = m_width;
    uint32_t h = m_height;

    for (uint32_t i = 0; i < mips; i++)
    {
        uint32_t levelSize = 0;
        if (compressed)
        {
            levelSize = ((w + 3) / 4) * ((h + 3) / 4) * block_size;
        }
        else
        {
            levelSize = w * h * 4;
        }

        VkBufferImageCopy region = {};
        region.bufferOffset = offset;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = i;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageExtent = { w, h, 1 };
        regions.push_back(region);

        offset += levelSize;
        w = std::max(1u, w / 2);
        h = std::max(1u, h / 2);

        if (offset > dataSize) break;
    }

    VulkanHW.TransitionImageLayout(m_image, m_format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mips);
    VulkanHW.CopyBufferToImage(stagingBuffer, m_image, regions);
    VulkanHW.TransitionImageLayout(m_image, m_format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mips);

    vmaDestroyBuffer(VulkanHW.GetAllocator(), stagingBuffer, stagingAllocation);
}

void CVulkanTexture::UploadData(void* data, uint32_t size)
{
    // Simplified upload for single mip level
    VkBuffer stagingBuffer;
    VmaAllocation stagingAllocation;
    VulkanHW.CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, stagingBuffer, stagingAllocation, VMA_ALLOCATION_CREATE_MAPPED_BIT);

    void* mappedData;
    vmaMapMemory(VulkanHW.GetAllocator(), stagingAllocation, &mappedData);
    memcpy(mappedData, data, size);
    vmaUnmapMemory(VulkanHW.GetAllocator(), stagingAllocation);

    VkBufferImageCopy region = {};
    region.bufferOffset = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageExtent = { m_width, m_height, 1 };

    xr_vector<VkBufferImageCopy> regions = { region };

    VulkanHW.TransitionImageLayout(m_image, m_format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1);
    VulkanHW.CopyBufferToImage(stagingBuffer, m_image, regions);
    VulkanHW.TransitionImageLayout(m_image, m_format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);

    vmaDestroyBuffer(VulkanHW.GetAllocator(), stagingBuffer, stagingAllocation);
}

void CVulkanTexture::CreateImageView()
{
    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = m_image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = m_format;

    if (m_format == VK_FORMAT_D32_SFLOAT || m_format == VK_FORMAT_D16_UNORM)
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    else if (m_format == VK_FORMAT_D32_SFLOAT_S8_UINT || m_format == VK_FORMAT_D24_UNORM_S8_UINT)
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    else
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = m_mips > 0 ? m_mips : 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(VulkanHW.GetDevice(), &viewInfo, nullptr, &m_image_view) != VK_SUCCESS)
    {
        Msg("! Vulkan: Failed to create image view!");
    }

    m_rt_view.image = m_image;
    m_rt_view.view = m_image_view;
    m_rt_view.format = m_format;
    m_rt_view.extent = { m_width, m_height };
    m_rt_view.current_layout = VK_IMAGE_LAYOUT_UNDEFINED;
}
