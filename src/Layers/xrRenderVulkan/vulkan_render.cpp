#include "stdafx.h"
#include "vulkan_render.h"

#ifndef _WIN32
#include <xcb/xcb.h>
#endif

#define VOLK_IMPLEMENTATION
#include "../../3rd party/volk/volk.h"

#define VMA_IMPLEMENTATION
#include "../../3rd party/vma/vk_mem_alloc.h"

CVulkanRender VulkanRenderImpl;

CVulkanRender::CVulkanRender()
{
    m_instance = VK_NULL_HANDLE;
    m_physical_device = VK_NULL_HANDLE;
    m_device = VK_NULL_HANDLE;
    m_surface = VK_NULL_HANDLE;
    m_swapchain = VK_NULL_HANDLE;
    m_render_pass = VK_NULL_HANDLE;
    m_command_pool = VK_NULL_HANDLE;
    m_image_available_semaphore = VK_NULL_HANDLE;
    m_render_finished_semaphore = VK_NULL_HANDLE;
    m_in_flight_fence = VK_NULL_HANDLE;
}

CVulkanRender::~CVulkanRender()
{
}

void CVulkanRender::create()
{
    Msg("Vulkan: Initializing renderer...");

    if (volkInitialize() != VK_SUCCESS)
    {
        Msg("! Vulkan: Failed to initialize volk!");
        return;
    }

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

    VkResult result = vkCreateInstance(&create_info, nullptr, &m_instance);
    if (result != VK_SUCCESS)
    {
        Msg("! Vulkan: Failed to create instance! Error code: %d", result);
        return;
    }

    volkLoadInstance(m_instance);

    Msg("Vulkan: Instance created.");

    // Surface creation
#ifdef _WIN32
    VkWin32SurfaceCreateInfoKHR surface_create_info = {};
    surface_create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surface_create_info.hinstance = GetModuleHandle(nullptr);
    surface_create_info.hwnd = Device.m_hWnd;
    result = vkCreateWin32SurfaceKHR(m_instance, &surface_create_info, nullptr, &m_surface);
#else
    VkXcbSurfaceCreateInfoKHR surface_create_info = {};
    surface_create_info.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
    surface_create_info.connection = (xcb_connection_t*)Device.m_XWindow; // Assuming connection is stored here for now
    surface_create_info.window = (xcb_window_t)(intptr_t)Device.m_hWnd;
    result = vkCreateXcbSurfaceKHR(m_instance, &surface_create_info, nullptr, &m_surface);
#endif

    if (result != VK_SUCCESS)
    {
        Msg("! Vulkan: Failed to create surface! Error code: %d", result);
        return;
    }

    // Physical device selection
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);
    if (deviceCount == 0)
    {
        Msg("! Vulkan: Failed to find GPUs with Vulkan support!");
        return;
    }

    xr_vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

    m_physical_device = devices[0]; // Just take the first one for now
    Msg("Vulkan: Physical device selected.");

    // Queue family selection
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_physical_device, &queueFamilyCount, nullptr);
    xr_vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(m_physical_device, &queueFamilyCount, queueFamilies.data());

    int graphicsFamily = -1;
    for (uint32_t i = 0; i < queueFamilyCount; i++)
    {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            graphicsFamily = i;
            break;
        }
    }

    if (graphicsFamily == -1)
    {
        Msg("! Vulkan: Failed to find a graphics queue family!");
        return;
    }

    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = graphicsFamily;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    VkPhysicalDeviceFeatures deviceFeatures = {};

    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

    xr_vector<const char*> deviceExtensions;
    deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    deviceCreateInfo.enabledExtensionCount = (uint32_t)deviceExtensions.size();
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

    result = vkCreateDevice(m_physical_device, &deviceCreateInfo, nullptr, &m_device);
    if (result != VK_SUCCESS)
    {
        Msg("! Vulkan: Failed to create logical device! Error code: %d", result);
        return;
    }

    vkGetDeviceQueue(m_device, graphicsFamily, 0, &m_graphics_queue);
    m_present_queue = m_graphics_queue; // Assume for now

    Msg("Vulkan: Logical device created.");

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

    result = vmaCreateAllocator(&allocatorInfo, &m_allocator);
    if (result != VK_SUCCESS)
    {
        Msg("! Vulkan: Failed to create VMA allocator! Error code: %d", result);
        return;
    }
    Msg("Vulkan: VMA Allocator created.");

    // Swapchain creation (skeleton)
    VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.surface = m_surface;
    swapchainCreateInfo.minImageCount = 2; // Double buffering
    swapchainCreateInfo.imageFormat = VK_FORMAT_B8G8R8A8_UNORM;
    swapchainCreateInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    swapchainCreateInfo.imageExtent = { Device.dwWidth, Device.dwHeight };
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    if (m_surface != VK_NULL_HANDLE)
    {
        result = vkCreateSwapchainKHR(m_device, &swapchainCreateInfo, nullptr, &m_swapchain);
        if (result != VK_SUCCESS)
        {
            Msg("! Vulkan: Failed to create swapchain! Error code: %d", result);
            return;
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

            result = vkCreateImageView(m_device, &viewInfo, nullptr, &m_swapchain_image_views[i]);
            if (result != VK_SUCCESS)
            {
                Msg("! Vulkan: Failed to create image view for swapchain! Error code: %d", result);
                return;
            }
        }

        Msg("Vulkan: Swapchain and Image Views created.");

        // Render Pass creation
        VkAttachmentDescription colorAttachment = {};
        colorAttachment.format = m_swapchain_format;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef = {};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;

        result = vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_render_pass);
        if (result != VK_SUCCESS)
        {
            Msg("! Vulkan: Failed to create render pass! Error code: %d", result);
            return;
        }

        // Framebuffer creation
        m_framebuffers.resize(m_swapchain_image_views.size());
        for (size_t i = 0; i < m_swapchain_image_views.size(); i++)
        {
            VkImageView attachments[] = { m_swapchain_image_views[i] };

            VkFramebufferCreateInfo framebufferInfo = {};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = m_render_pass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = m_swapchain_extent.width;
            framebufferInfo.height = m_swapchain_extent.height;
            framebufferInfo.layers = 1;

            result = vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &m_framebuffers[i]);
            if (result != VK_SUCCESS)
            {
                Msg("! Vulkan: Failed to create framebuffer! Error code: %d", result);
                return;
            }
        }
        Msg("Vulkan: Render Pass and Framebuffers created.");

        // Command Pool creation
        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = graphicsFamily;
        poolInfo.flags = 0; // Optional

        result = vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_command_pool);
        if (result != VK_SUCCESS)
        {
            Msg("! Vulkan: Failed to create command pool! Error code: %d", result);
            return;
        }

        // Command Buffer allocation
        m_command_buffers.resize(m_framebuffers.size());

        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_command_pool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t)m_command_buffers.size();

        result = vkAllocateCommandBuffers(m_device, &allocInfo, m_command_buffers.data());
        if (result != VK_SUCCESS)
        {
            Msg("! Vulkan: Failed to allocate command buffers! Error code: %d", result);
            return;
        }
        Msg("Vulkan: Command Pool and Buffers created.");

        // Synchronization primitives
        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        if (vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_image_available_semaphore) != VK_SUCCESS ||
            vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_render_finished_semaphore) != VK_SUCCESS ||
            vkCreateFence(m_device, &fenceInfo, nullptr, &m_in_flight_fence) != VK_SUCCESS)
        {
            Msg("! Vulkan: Failed to create synchronization primitives!");
            return;
        }
        Msg("Vulkan: Synchronization primitives created.");
    }
}

void CVulkanRender::destroy()
{
    if (m_in_flight_fence != VK_NULL_HANDLE)
    {
        vkDestroyFence(m_device, m_in_flight_fence, nullptr);
        m_in_flight_fence = VK_NULL_HANDLE;
    }

    if (m_render_finished_semaphore != VK_NULL_HANDLE)
    {
        vkDestroySemaphore(m_device, m_render_finished_semaphore, nullptr);
        m_render_finished_semaphore = VK_NULL_HANDLE;
    }

    if (m_image_available_semaphore != VK_NULL_HANDLE)
    {
        vkDestroySemaphore(m_device, m_image_available_semaphore, nullptr);
        m_image_available_semaphore = VK_NULL_HANDLE;
    }

    if (m_command_pool != VK_NULL_HANDLE)
    {
        vkDestroyCommandPool(m_device, m_command_pool, nullptr);
        m_command_pool = VK_NULL_HANDLE;
    }

    for (auto framebuffer : m_framebuffers)
    {
        vkDestroyFramebuffer(m_device, framebuffer, nullptr);
    }
    m_framebuffers.clear();

    if (m_render_pass != VK_NULL_HANDLE)
    {
        vkDestroyRenderPass(m_device, m_render_pass, nullptr);
        m_render_pass = VK_NULL_HANDLE;
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

    if (m_surface != VK_NULL_HANDLE)
    {
        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
        m_surface = VK_NULL_HANDLE;
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

    if (m_instance != VK_NULL_HANDLE)
    {
        vkDestroyInstance(m_instance, nullptr);
        m_instance = VK_NULL_HANDLE;
    }
}

void CVulkanRender::reset_begin()
{
}

void CVulkanRender::reset_end()
{
}

void CVulkanRender::level_Load(IReader* fs)
{
}

void CVulkanRender::level_Unload()
{
}

HRESULT CVulkanRender::shader_compile(
    LPCSTR name,
    DWORD const* pSrcData,
    UINT SrcDataLen,
    LPCSTR pFunctionName,
    LPCSTR pTarget,
    DWORD Flags,
    void*& result)
{
    return E_NOTIMPL;
}
