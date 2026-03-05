#include "stdafx.h"
#include "vulkan_render.h"

CVulkanRender VulkanRenderImpl;

CVulkanRender::CVulkanRender()
{
    m_instance = VK_NULL_HANDLE;
    m_physical_device = VK_NULL_HANDLE;
    m_device = VK_NULL_HANDLE;
}

CVulkanRender::~CVulkanRender()
{
}

void CVulkanRender::create()
{
    Msg("Vulkan: Initializing renderer...");

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

    // In a real implementation, we would call vkCreateInstance here.
    // For now, we stub it to allow compilation without linking to the Vulkan loader.
    /*
    VkResult result = vkCreateInstance(&create_info, nullptr, &m_instance);
    if (result != VK_SUCCESS)
    {
        Msg("! Vulkan: Failed to create instance!");
        return;
    }
    */

    Msg("Vulkan: Instance created (stub)");

    // Physical device selection logic would go here.
    Msg("Vulkan: Selecting physical device (stub)");
}

void CVulkanRender::destroy()
{
    if (m_device != VK_NULL_HANDLE)
    {
        // vkDestroyDevice(m_device, nullptr);
        m_device = VK_NULL_HANDLE;
    }

    if (m_instance != VK_NULL_HANDLE)
    {
        // vkDestroyInstance(m_instance, nullptr);
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
