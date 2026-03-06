#pragma once

#ifndef _WIN32
#define VK_USE_PLATFORM_XCB_KHR
#endif
#include "../../3rd party/volk/volk.h"
#include "../../3rd party/vma/vk_mem_alloc.h"

class CVulkanHW
{
public:
    CVulkanHW();
    ~CVulkanHW();

    void Create();
    void Destroy();
    void RecreateSwapchain();

    VkInstance GetInstance() { return m_instance; }
    VkPhysicalDevice GetPhysicalDevice() { return m_physical_device; }
    VkDevice GetDevice() { return m_device; }
    VkQueue GetGraphicsQueue() { return m_graphics_queue; }
    VkQueue GetPresentQueue() { return m_present_queue; }
    VkSurfaceKHR GetSurface() { return m_surface; }
    VkSwapchainKHR GetSwapchain() { return m_swapchain; }
    VkFormat GetSwapchainFormat() { return m_swapchain_format; }
    VkExtent2D GetSwapchainExtent() { return m_swapchain_extent; }
    const xr_vector<VkImage>& GetSwapchainImages() { return m_swapchain_images; }
    const xr_vector<VkImageView>& GetSwapchainImageViews() { return m_swapchain_image_views; }
    VmaAllocator GetAllocator() { return m_allocator; }
    int GetGraphicsFamily() { return m_graphics_family; }
    int GetPresentFamily() { return m_present_family; }

    VkImage GetDepthImage() { return m_depth_image; }
    VkImageView GetDepthImageView() { return m_depth_image_view; }
    VkFormat GetDepthFormat() { return m_depth_format; }

private:
    VkInstance m_instance;
    VkPhysicalDevice m_physical_device;
    VkDevice m_device;
    VkQueue m_graphics_queue;
    VkQueue m_present_queue;
    VkSurfaceKHR m_surface;
    VkSwapchainKHR m_swapchain;
    VkFormat m_swapchain_format;
    VkExtent2D m_swapchain_extent;
    xr_vector<VkImage> m_swapchain_images;
    xr_vector<VkImageView> m_swapchain_image_views;

    VkImage m_depth_image;
    VmaAllocation m_depth_allocation;
    VkImageView m_depth_image_view;
    VkFormat m_depth_format;

    VmaAllocator m_allocator;
    int m_graphics_family;
    int m_present_family;

    void CreateInstance();
    void CreateSurface();
    void SelectPhysicalDevice();
    void CreateLogicalDevice();
    void CreateSwapchain();
    void CreateDepthResources();
    VkFormat FindSupportedFormat(const xr_vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

    void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage mem_usage, VkBuffer& buffer, VmaAllocation& allocation, VmaAllocationCreateFlags flags = 0);
    void CreateTexture(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, VkImage& image, VmaAllocation& allocation, VkImageView& view);

#ifdef DEBUG
    VkDebugUtilsMessengerEXT m_debug_messenger;
    void SetupDebugMessenger();
#endif
};

extern CVulkanHW VulkanHW;
