#include "stdafx.h"
#include "vulkan_render.h"

CVulkanRender VulkanRenderImpl;

CVulkanRender::CVulkanRender()
{
    Target = nullptr;
}

CVulkanRender::~CVulkanRender()
{
}

void CVulkanRender::create()
{
    VulkanHW.Create();
    VulkanBackend.Create();
    Target = xr_new<CVulkanRenderTarget>();
}

void CVulkanRender::destroy()
{
    xr_delete(Target);
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
    Lights.Update();
}

void CVulkanRender::Render()
{
    if (!Begin()) return;

    // Shadow passes
    // Lights.package.sort();
    for (light* L : Lights.package.v_shadowed)
    {
        Target->phase_smap_direct();
        r_dsgraph_render_graph(0);
        Target->phase_scene_end();
    }

    // G-Buffer pass
    Target->phase_scene_begin();
    r_dsgraph_render_graph(0);
    Target->phase_scene_end();

    // Lighting pass
    Target->phase_accumulator();
    for (light* L : Lights.package.v_point)
    {
        // TODO: Render point light volume
    }
    for (light* L : Lights.package.v_spot)
    {
        // TODO: Render spot light volume
    }
    for (light* L : Lights.package.v_shadowed)
    {
        // TODO: Render shadowed light volume
    }
    Target->phase_scene_end();

    // Final combine
    Target->phase_combine();
    // Render glows
    Glows.Render();
    // Render fullscreen quad here
    Target->phase_scene_end();

    // Post-processing graph
    Target->phase_bloom();
    Target->phase_scene_end();

    Target->phase_sunshafts();
    Target->phase_scene_end();

    Target->phase_dof();
    Target->phase_scene_end();

    End();
}

void CVulkanRender::add_Visual(IRenderVisual* V)
{
}

void CVulkanRender::add_Geometry(IRenderVisual* V)
{
}

IRender_Glow* CVulkanRender::glow_create()
{
    return (IRender_Glow*)xr_new<CVulkanGlow>();
}
