#include "stdafx.h"
#include "VulkanTexture.h"

CVulkanTexture::CVulkanTexture()
{
    m_image = VK_NULL_HANDLE;
    m_allocation = VK_NULL_HANDLE;
    m_image_view = VK_NULL_HANDLE;
    m_format = VK_FORMAT_UNDEFINED;
    m_width = 0;
    m_height = 0;
}

CVulkanTexture::~CVulkanTexture()
{
    Destroy();
}

void CVulkanTexture::Create(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage)
{
    m_width = width;
    m_height = height;
    m_format = format;

    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
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
}

void CVulkanTexture::Destroy()
{
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

void CVulkanTexture::Load(LPCSTR name)
{
    // Skeleton implementation
    Msg("Vulkan: Loading texture %s", name);
}

void CVulkanTexture::CreateImageView()
{
    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = m_image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = m_format;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(VulkanHW.GetDevice(), &viewInfo, nullptr, &m_image_view) != VK_SUCCESS)
    {
        Msg("! Vulkan: Failed to create image view!");
    }
}
