#include "stdafx.h"
#include "vulkan_render.h"
#include "../xrRender/du_sphere.h"
#include "../xrRender/du_cone.h"

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
    Device.Statistic->RenderCALC.Begin();

    // traverse sector/portal structure
    PortalTraverser.traverse
    (
        detectSector(Device.vCameraPosition),
        ViewBase,
        Device.vCameraPosition,
        Device.mFullTransform,
        CPortalTraverser::VQ_HOM + CPortalTraverser::VQ_FADE
    );

    // build render-graph
    Device.Statistic->RenderDUMP_RT_W.Begin();
    r_dsgraph_build();
    Device.Statistic->RenderDUMP_RT_W.End();

    Lights.Update();

    Device.Statistic->RenderCALC.End();
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
        L->xform_calc();
        VulkanBackend.set_Geometry(Target->g_accum_point._get());
        VulkanBackend.DrawIndexed(DU_SPHERE_NUMFACES * 3);
    }
    for (light* L : Lights.package.v_spot)
    {
        L->xform_calc();
        VulkanBackend.set_Geometry(Target->g_accum_spot._get());
        VulkanBackend.DrawIndexed(DU_CONE_NUMFACES * 3);
    }
    for (light* L : Lights.package.v_shadowed)
    {
        L->xform_calc();
        if (L->flags.type == IRender_Light::POINT)
        {
            VulkanBackend.set_Geometry(Target->g_accum_point._get());
            VulkanBackend.DrawIndexed(DU_SPHERE_NUMFACES * 3);
        }
        else
        {
            VulkanBackend.set_Geometry(Target->g_accum_spot._get());
            VulkanBackend.DrawIndexed(DU_CONE_NUMFACES * 3);
        }
    }
    Target->phase_scene_end();

    // Final combine
    Target->phase_combine();
    // Render environment (sky, clouds)
    g_pGamePersistent->Environment().RenderSky();
    g_pGamePersistent->Environment().RenderClouds();

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
    add_leafs_Dynamic((dxRender_Visual*)V);
}

void CVulkanRender::add_Geometry(IRenderVisual* V)
{
    add_Static((dxRender_Visual*)V, View->getMask());
}

IRender_Glow* CVulkanRender::glow_create()
{
    return (IRender_Glow*)xr_new<CVulkanGlow>();
}
