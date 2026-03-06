#include "stdafx.h"
#include "vulkan_render.h"

CVulkanRender VulkanRenderImpl;

CVulkanRender::CVulkanRender()
{
}

CVulkanRender::~CVulkanRender()
{
}

void CVulkanRender::create()
{
    VulkanHW.Create();
    VulkanBackend.Create();
}

void CVulkanRender::destroy()
{
    VulkanBackend.Destroy();
    VulkanHW.Destroy();
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
