#include "stdafx.h"
#include "VulkanHW.h"

#ifndef _WIN32
#include <xcb/xcb.h>
#endif

#define VOLK_IMPLEMENTATION
#include "../../3rd party/volk/volk.h"

#define VMA_IMPLEMENTATION
#include "../../3rd party/vma/vk_mem_alloc.h"

CVulkanHW VulkanHW;

CVulkanHW::CVulkanHW()
{
    m_instance = VK_NULL_HANDLE;
    m_physical_device = VK_NULL_HANDLE;
    m_device = VK_NULL_HANDLE;
    m_surface = VK_NULL_HANDLE;
    m_swapchain = VK_NULL_HANDLE;
    m_depth_image = VK_NULL_HANDLE;
    m_depth_image_view = VK_NULL_HANDLE;
    m_depth_allocation = VK_NULL_HANDLE;
    m_allocator = VK_NULL_HANDLE;
    m_graphics_family = -1;
}

CVulkanHW::~CVulkanHW()
{
}

void CVulkanHW::Create()
{
    Msg("Vulkan: Initializing Hardware...");

    if (volkInitialize() != VK_SUCCESS)
    {
        Msg("! Vulkan: Failed to initialize volk!");
        return;
    }

    CreateInstance();
    volkLoadInstance(m_instance);
    CreateSurface();
    SelectPhysicalDevice();
    CreateLogicalDevice();

    // Initialize VMA
    VmaVulkanFunctions vma_vulkan_func = {};
    vma_vulkan_func.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
    vma_vulkan_func.vkGetDeviceProcAddr = vkGetDeviceProcAddr;

    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_0;
    allocatorInfo.physicalDevice = m_physical_device;
    allocatorInfo.device = m_device;
    allocatorInfo.instance = m_instance;
    allocatorInfo.pVulkanFunctions = &vma_vulkan_func;

    if (vmaCreateAllocator(&allocatorInfo, &m_allocator) != VK_SUCCESS)
    {
        Msg("! Vulkan: Failed to create VMA allocator!");
        return;
    }

    CreateSwapchain();
    CreateDepthResources();
}

void CVulkanHW::Destroy()
{
    if (m_depth_image_view != VK_NULL_HANDLE)
    {
        vkDestroyImageView(m_device, m_depth_image_view, nullptr);
        m_depth_image_view = VK_NULL_HANDLE;
    }

    if (m_depth_image != VK_NULL_HANDLE)
    {
        vmaDestroyImage(m_allocator, m_depth_image, m_depth_allocation);
        m_depth_image = VK_NULL_HANDLE;
    }

    for (auto imageView : m_swapchain_image_views)
    {
        vkDestroyImageView(m_device, imageView, nullptr);
    }
    m_swapchain_image_views.clear();

    if (m_swapchain != VK_NULL_HANDLE)
    {
        vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
        m_swapchain = VK_NULL_HANDLE;
    }

    if (m_allocator != VK_NULL_HANDLE)
    {
        vmaDestroyAllocator(m_allocator);
        m_allocator = VK_NULL_HANDLE;
    }

    if (m_device != VK_NULL_HANDLE)
    {
        vkDestroyDevice(m_device, nullptr);
        m_device = VK_NULL_HANDLE;
    }

    if (m_surface != VK_NULL_HANDLE)
    {
        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
        m_surface = VK_NULL_HANDLE;
    }

    if (m_instance != VK_NULL_HANDLE)
    {
        vkDestroyInstance(m_instance, nullptr);
        m_instance = VK_NULL_HANDLE;
    }
}

void CVulkanHW::RecreateSwapchain()
{
    vkDeviceWaitIdle(m_device);

    VulkanBackend.OnDeviceDestroy();

    if (m_depth_image_view != VK_NULL_HANDLE)
    {
        vkDestroyImageView(m_device, m_depth_image_view, nullptr);
        m_depth_image_view = VK_NULL_HANDLE;
    }

    if (m_depth_image != VK_NULL_HANDLE)
    {
        vmaDestroyImage(m_allocator, m_depth_image, m_depth_allocation);
        m_depth_image = VK_NULL_HANDLE;
    }

    for (auto imageView : m_swapchain_image_views)
    {
        vkDestroyImageView(m_device, imageView, nullptr);
    }
    m_swapchain_image_views.clear();

    if (m_swapchain != VK_NULL_HANDLE)
    {
        vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
        m_swapchain = VK_NULL_HANDLE;
    }

    CreateSwapchain();
    CreateDepthResources();
    VulkanBackend.OnDeviceCreate();
}

void CVulkanHW::CreateInstance()
{
    VkApplicationInfo app_info = {};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "X-Ray Engine";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "X-Ray";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;

    xr_vector<const char*> extensions;
#ifdef _WIN32
    extensions.push_back("VK_KHR_win32_surface");
#else
    extensions.push_back("VK_KHR_xcb_surface");
#endif
    extensions.push_back("VK_KHR_surface");

    create_info.enabledExtensionCount = (uint32_t)extensions.size();
    create_info.ppEnabledExtensionNames = extensions.data();

    if (vkCreateInstance(&create_info, nullptr, &m_instance) != VK_SUCCESS)
    {
        Msg("! Vulkan: Failed to create instance!");
    }
}

void CVulkanHW::CreateSurface()
{
#ifdef _WIN32
    VkWin32SurfaceCreateInfoKHR surface_create_info = {};
    surface_create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surface_create_info.hinstance = GetModuleHandle(nullptr);
    surface_create_info.hwnd = Device.m_hWnd;
    if (vkCreateWin32SurfaceKHR(m_instance, &surface_create_info, nullptr, &m_surface) != VK_SUCCESS)
    {
        Msg("! Vulkan: Failed to create Win32 surface!");
    }
#else
    VkXcbSurfaceCreateInfoKHR surface_create_info = {};
    surface_create_info.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
    surface_create_info.connection = (xcb_connection_t*)Device.m_XWindow;
    surface_create_info.window = (xcb_window_t)(intptr_t)Device.m_hWnd;
    if (vkCreateXcbSurfaceKHR(m_instance, &surface_create_info, nullptr, &m_surface) != VK_SUCCESS)
    {
        Msg("! Vulkan: Failed to create XCB surface!");
    }
#endif
}

void CVulkanHW::SelectPhysicalDevice()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);

    if (deviceCount == 0)
    {
        Msg("! Vulkan: No physical devices found!");
        return;
    }

    xr_vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

    auto RateDevice = [](VkPhysicalDevice device) {
        int score = 0;
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(device, &properties);
        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceFeatures(device, &features);

        if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            score += 1000;

        score += properties.limits.maxImageDimension2D;

        if (!features.geometryShader)
            return 0;

        return score;
    };

    int maxScore = -1;
    for (const auto& device : devices)
    {
        int score = RateDevice(device);
        if (score > maxScore)
        {
            maxScore = score;
            m_physical_device = device;
        }
    }

    if (m_physical_device == VK_NULL_HANDLE)
    {
        Msg("! Vulkan: Failed to find a suitable physical device!");
    }
    else
    {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(m_physical_device, &properties);
        Msg("Vulkan: Selected physical device: %s", properties.deviceName);
    }
}

void CVulkanHW::CreateLogicalDevice()
{
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_physical_device, &queueFamilyCount, nullptr);
    xr_vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(m_physical_device, &queueFamilyCount, queueFamilies.data());

    m_graphics_family = -1;
    for (uint32_t i = 0; i < queueFamilyCount; i++)
    {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            m_graphics_family = i;
            break;
        }
    }

    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = m_graphics_family;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    VkPhysicalDeviceFeatures deviceFeatures = {};
    xr_vector<const char*> deviceExtensions;
    deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
    deviceCreateInfo.enabledExtensionCount = (uint32_t)deviceExtensions.size();
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (vkCreateDevice(m_physical_device, &deviceCreateInfo, nullptr, &m_device) != VK_SUCCESS)
    {
        Msg("! Vulkan: Failed to create logical device!");
    }

    vkGetDeviceQueue(m_device, graphicsFamily, 0, &m_graphics_queue);
    m_present_queue = m_graphics_queue;
}

void CVulkanHW::CreateSwapchain()
{
    VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.surface = m_surface;
    swapchainCreateInfo.minImageCount = 2;
    swapchainCreateInfo.imageFormat = VK_FORMAT_B8G8R8A8_UNORM;
    swapchainCreateInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    swapchainCreateInfo.imageExtent = { Device.dwWidth, Device.dwHeight };
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    if (vkCreateSwapchainKHR(m_device, &swapchainCreateInfo, nullptr, &m_swapchain) != VK_SUCCESS)
    {
        Msg("! Vulkan: Failed to create swapchain!");
    }

    m_swapchain_format = swapchainCreateInfo.imageFormat;
    m_swapchain_extent = swapchainCreateInfo.imageExtent;

    uint32_t imageCount;
    vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, nullptr);
    m_swapchain_images.resize(imageCount);
    vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, m_swapchain_images.data());

    m_swapchain_image_views.resize(imageCount);
    for (uint32_t i = 0; i < imageCount; i++)
    {
        VkImageViewCreateInfo viewInfo = {};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = m_swapchain_images[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = m_swapchain_format;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(m_device, &viewInfo, nullptr, &m_swapchain_image_views[i]) != VK_SUCCESS)
        {
            Msg("! Vulkan: Failed to create image view!");
        }
    }
}

void CVulkanHW::CreateDepthResources()
{
    m_depth_format = FindSupportedFormat(
        { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );

    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = m_swapchain_extent.width;
    imageInfo.extent.height = m_swapchain_extent.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = m_depth_format;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    if (vmaCreateImage(m_allocator, &imageInfo, &allocInfo, &m_depth_image, &m_depth_allocation, nullptr) != VK_SUCCESS)
    {
        Msg("! Vulkan: Failed to create depth image!");
    }

    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = m_depth_image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = m_depth_format;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(m_device, &viewInfo, nullptr, &m_depth_image_view) != VK_SUCCESS)
    {
        Msg("! Vulkan: Failed to create depth image view!");
    }
}

VkFormat CVulkanHW::FindSupportedFormat(const xr_vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
    for (VkFormat format : candidates)
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(m_physical_device, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
        {
            return format;
        }
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
        {
            return format;
        }
    }

    return VK_FORMAT_UNDEFINED;
}
