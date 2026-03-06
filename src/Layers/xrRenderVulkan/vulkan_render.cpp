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

ID3DBaseTexture* CVulkanRender::texture_load(LPCSTR fname, u32& msize)
{
    CVulkanTexture* T = xr_new<CVulkanTexture>();
    T->Load(fname, msize);
    return T;
}

void CVulkanRender::Calculate()
{
}

void CVulkanRender::Render()
{
    if (!Begin()) return;

    // TODO: Implement render graph processing

    End();
}

void CVulkanRender::add_Visual(IRenderVisual* V)
{
}

void CVulkanRender::add_Geometry(IRenderVisual* V)
{
}
