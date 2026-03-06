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
    m_present_family = -1;
#ifdef DEBUG
    m_debug_messenger = VK_NULL_HANDLE;
#endif
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
#ifdef DEBUG
    SetupDebugMessenger();
#endif
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
#ifdef DEBUG
        if (m_debug_messenger != VK_NULL_HANDLE)
        {
            vkDestroyDebugUtilsMessengerEXT(m_instance, m_debug_messenger, nullptr);
            m_debug_messenger = VK_NULL_HANDLE;
        }
#endif
        vkDestroyInstance(m_instance, nullptr);
        m_instance = VK_NULL_HANDLE;
    }
}

void CVulkanHW::RecreateSwapchain()
{
    // Handle minimization
    RECT rc;
    GetClientRect((HWND)Device.m_hWnd, &rc);
    while (rc.right == 0 || rc.bottom == 0)
    {
        GetClientRect((HWND)Device.m_hWnd, &rc);
        Sleep(1);
    }

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

#ifdef DEBUG
static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    Msg("! [Vulkan Validation]: %s", pCallbackData->pMessage);
    return VK_FALSE;
}
#endif

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
#ifdef DEBUG
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

    create_info.enabledExtensionCount = (uint32_t)extensions.size();
    create_info.ppEnabledExtensionNames = extensions.data();

    xr_vector<const char*> layers;
#ifdef DEBUG
    layers.push_back("VK_LAYER_KHRONOS_validation");
#endif
    create_info.enabledLayerCount = (uint32_t)layers.size();
    create_info.ppEnabledLayerNames = layers.data();

    if (vkCreateInstance(&create_info, nullptr, &m_instance) != VK_SUCCESS)
    {
        Msg("! Vulkan: Failed to create instance!");
    }
}

void CVulkanHW::CreateSurface()
{
#ifdef _WIN32
    if (Device.m_hWnd == NULL)
    {
        Msg("! Vulkan: Device.m_hWnd is NULL during surface creation!");
        return;
    }
    VkWin32SurfaceCreateInfoKHR surface_create_info = {};
    surface_create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surface_create_info.hinstance = GetModuleHandle(nullptr);
    surface_create_info.hwnd = Device.m_hWnd;
    if (vkCreateWin32SurfaceKHR(m_instance, &surface_create_info, nullptr, &m_surface) != VK_SUCCESS)
    {
        Msg("! Vulkan: Failed to create Win32 surface!");
    }
#else
    if (Device.m_hWnd == NULL)
    {
        Msg("! Vulkan: Device.m_hWnd is NULL during surface creation!");
        return;
    }
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

        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
        xr_vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        bool swapchainSupported = false;
        for (const auto& extension : availableExtensions)
        {
            if (xr_strcmp(extension.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
            {
                swapchainSupported = true;
                break;
            }
        }

        if (!swapchainSupported)
            return 0;

        // Check for queue support
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
        xr_vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        bool graphicsSupported = false;
        bool presentSupported = false;

        for (uint32_t i = 0; i < queueFamilyCount; i++)
        {
            if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                graphicsSupported = true;

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &presentSupport);
            if (presentSupport)
                presentSupported = true;
        }

        if (!graphicsSupported || !presentSupported)
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
    m_present_family = -1;

    for (uint32_t i = 0; i < queueFamilyCount; i++)
    {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            m_graphics_family = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(m_physical_device, i, m_surface, &presentSupport);
        if (presentSupport)
        {
            m_present_family = i;
        }

        if (m_graphics_family != -1 && m_present_family != -1)
            break;
    }

    xr_set<uint32_t> uniqueQueueFamilies = { (uint32_t)m_graphics_family, (uint32_t)m_present_family };
    xr_vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    float queuePriority = 1.0f;

    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures = {};
    xr_vector<const char*> deviceExtensions;
    deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = (uint32_t)queueCreateInfos.size();
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
    deviceCreateInfo.enabledExtensionCount = (uint32_t)deviceExtensions.size();
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (vkCreateDevice(m_physical_device, &deviceCreateInfo, nullptr, &m_device) != VK_SUCCESS)
    {
        Msg("! Vulkan: Failed to create logical device!");
    }

    vkGetDeviceQueue(m_device, m_graphics_family, 0, &m_graphics_queue);
    vkGetDeviceQueue(m_device, m_present_family, 0, &m_present_queue);
}

void CVulkanHW::CreateSwapchain()
{
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physical_device, m_surface, &capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_physical_device, m_surface, &formatCount, nullptr);
    xr_vector<VkSurfaceFormatKHR> formats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_physical_device, m_surface, &formatCount, formats.data());

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(m_physical_device, m_surface, &presentModeCount, nullptr);
    xr_vector<VkPresentModeKHR> presentModes(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(m_physical_device, m_surface, &presentModeCount, presentModes.data());

    VkSurfaceFormatKHR surfaceFormat = formats[0];
    for (const auto& format : formats)
    {
        if (format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            surfaceFormat = format;
            break;
        }
    }

    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    if (!psDeviceFlags.test(rsVSync))
    {
        for (const auto& mode : presentModes)
        {
            if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                presentMode = mode;
                break;
            }
            if (mode == VK_PRESENT_MODE_IMMEDIATE_KHR)
            {
                presentMode = mode;
            }
        }
    }

    VkExtent2D extent = { Device.dwWidth, Device.dwHeight };
    extent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, extent.width));
    extent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, extent.height));

    uint32_t imageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
    {
        imageCount = capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.surface = m_surface;
    swapchainCreateInfo.minImageCount = imageCount;
    swapchainCreateInfo.imageFormat = surfaceFormat.format;
    swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
    swapchainCreateInfo.imageExtent = extent;
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    uint32_t queueFamilyIndices[] = { (uint32_t)m_graphics_family, (uint32_t)m_present_family };

    if (m_graphics_family != m_present_family)
    {
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainCreateInfo.queueFamilyIndexCount = 2;
        swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    swapchainCreateInfo.preTransform = capabilities.currentTransform;
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfo.presentMode = presentMode;
    swapchainCreateInfo.clipped = VK_TRUE;

    if (vkCreateSwapchainKHR(m_device, &swapchainCreateInfo, nullptr, &m_swapchain) != VK_SUCCESS)
    {
        Msg("! Vulkan: Failed to create swapchain!");
    }

    m_swapchain_format = surfaceFormat.format;
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

#ifdef DEBUG
void CVulkanHW::SetupDebugMessenger()
{
    VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = DebugCallback;

    if (vkCreateDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &m_debug_messenger) != VK_SUCCESS)
    {
        Msg("! Vulkan: Failed to set up debug messenger!");
    }
}
#endif
