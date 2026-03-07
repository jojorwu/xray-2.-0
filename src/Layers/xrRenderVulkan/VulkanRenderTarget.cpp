#include "stdafx.h"
#include "VulkanRenderTarget.h"
#include "VulkanBackend.h"

CVulkanRenderTarget::CVulkanRenderTarget()
{
    dwWidth = Device.dwWidth;
    dwHeight = Device.dwHeight;

    // Create G-Buffer targets
    rt_Position.create("$user$position", dwWidth, dwHeight, D3DFMT_A16B16G16R16F);
    rt_Normal.create("$user$normal", dwWidth, dwHeight, D3DFMT_A16B16G16R16F);
    rt_Color.create("$user$color", dwWidth, dwHeight, D3DFMT_A16B16G16R16F);

    // Create Accumulator target
    rt_Accumulator.create("$user$accumulator", dwWidth, dwHeight, D3DFMT_A16B16G16R16F);

    // Create Post-process targets
    rt_Bloom_1.create("$user$bloom1", dwWidth / 4, dwHeight / 4, D3DFMT_A8R8G8B8);
    rt_Bloom_2.create("$user$bloom2", dwWidth / 4, dwHeight / 4, D3DFMT_A8R8G8B8);
    rt_DOF.create("$user$dof", dwWidth, dwHeight, D3DFMT_A8R8G8B8);
    rt_Sunshafts_0.create("$user$sunshafts0", dwWidth, dwHeight, D3DFMT_A8R8G8B8);
    rt_Sunshafts_1.create("$user$sunshafts1", dwWidth, dwHeight, D3DFMT_A8R8G8B8);
    rt_Generic_0.create("$user$generic0", dwWidth, dwHeight, D3DFMT_A8R8G8B8);
    rt_Generic_1.create("$user$generic1", dwWidth, dwHeight, D3DFMT_A8R8G8B8);

    // Create shadow targets
    rt_smap_depth.create("$user$smap_depth", 2048, 2048, D3DFMT_D24S8);
    rt_smap_surf.create("$user$smap_surf", 2048, 2048, D3DFMT_R32F);
}

CVulkanRenderTarget::~CVulkanRenderTarget()
{
    rt_smap_surf.destroy();
    rt_smap_depth.destroy();
    rt_Generic_1.destroy();
    rt_Generic_0.destroy();
    rt_Sunshafts_1.destroy();
    rt_Sunshafts_0.destroy();
    rt_DOF.destroy();
    rt_Bloom_2.destroy();
    rt_Bloom_1.destroy();
    rt_Accumulator.destroy();
    rt_Color.destroy();
    rt_Normal.destroy();
    rt_Position.destroy();
}

void CVulkanRenderTarget::phase_scene_prepare()
{
    VulkanBackend.set_RT(nullptr, 0);
    VulkanBackend.set_RT(nullptr, 1);
    VulkanBackend.set_RT(nullptr, 2);
    VulkanBackend.set_ZB(VulkanHW.GetDepthRTView());
}

void CVulkanRenderTarget::phase_bloom()
{
    // Bloom build pass
    VulkanBackend.set_RT(rt_Bloom_1->pRT, 0);
    VulkanBackend.set_RT(nullptr, 1);
    VulkanBackend.set_RT(nullptr, 2);
    VulkanBackend.set_ZB(nullptr);
    VulkanBackend.ClearTarget();
    // Render downsampled scene here

    // Horizontal blur
    VulkanBackend.set_RT(rt_Bloom_2->pRT, 0);
    VulkanBackend.ClearTarget();
    // Render blur here

    // Vertical blur
    VulkanBackend.set_RT(rt_Bloom_1->pRT, 0);
    VulkanBackend.ClearTarget();
    // Render blur here
}

void CVulkanRenderTarget::phase_dof()
{
    VulkanBackend.set_RT(rt_DOF->pRT, 0);
    VulkanBackend.set_RT(nullptr, 1);
    VulkanBackend.set_RT(nullptr, 2);
    VulkanBackend.set_ZB(nullptr);
    VulkanBackend.ClearTarget();
    // Render DOF here
}

void CVulkanRenderTarget::phase_smap_direct()
{
    VulkanBackend.set_RT(rt_smap_surf->pRT, 0);
    VulkanBackend.set_RT(nullptr, 1);
    VulkanBackend.set_RT(nullptr, 2);
    VulkanBackend.set_ZB(rt_smap_depth->pRT); // depth RT is CVulkanRTView
    VulkanBackend.Clear();
}

void CVulkanRenderTarget::phase_smap_spot()
{
    VulkanBackend.set_RT(rt_smap_surf->pRT, 0);
    VulkanBackend.set_RT(nullptr, 1);
    VulkanBackend.set_RT(nullptr, 2);
    VulkanBackend.set_ZB(rt_smap_depth->pRT);
    VulkanBackend.Clear();
}

void CVulkanRenderTarget::phase_sunshafts()
{
    // Sunshafts mask generation
    VulkanBackend.set_RT(rt_Sunshafts_0->pRT, 0);
    VulkanBackend.set_RT(nullptr, 1);
    VulkanBackend.set_RT(nullptr, 2);
    VulkanBackend.set_ZB(VulkanHW.GetDepthRTView());
    VulkanBackend.ClearTarget();
    // Render mask

    // Sunshafts blur passes
    VulkanBackend.set_RT(rt_Sunshafts_1->pRT, 0);
    VulkanBackend.ClearTarget();
    // Pass 1

    VulkanBackend.set_RT(rt_Sunshafts_0->pRT, 0);
    VulkanBackend.ClearTarget();
    // Pass 2

    VulkanBackend.set_RT(rt_Sunshafts_1->pRT, 0);
    VulkanBackend.ClearTarget();
    // Pass 3
}

void CVulkanRenderTarget::phase_scene_begin()
{
    VulkanBackend.set_RT(rt_Position->pRT, 0);
    VulkanBackend.set_RT(rt_Normal->pRT, 1);
    VulkanBackend.set_RT(rt_Color->pRT, 2);
    VulkanBackend.set_ZB(VulkanHW.GetDepthRTView());
    VulkanBackend.Clear();
}

void CVulkanRenderTarget::phase_scene_end()
{
    VulkanBackend.EndRenderPass();
}

void CVulkanRenderTarget::phase_accumulator()
{
    VulkanBackend.set_RT(rt_Accumulator->pRT, 0);
    VulkanBackend.set_RT(nullptr, 1);
    VulkanBackend.set_RT(nullptr, 2);
    VulkanBackend.set_ZB(VulkanHW.GetDepthRTView());
    VulkanBackend.ClearTarget();
}

void CVulkanRenderTarget::phase_combine()
{
    VulkanBackend.set_RT(VulkanHW.GetSwapchainRTView(VulkanBackend.GetCurrentImageIndex()), 0);
    VulkanBackend.set_RT(nullptr, 1);
    VulkanBackend.set_RT(nullptr, 2);
    VulkanBackend.set_ZB(VulkanHW.GetDepthRTView());
    // In actual implementation, we'd bind rt_Position, rt_Normal, rt_Color, rt_Accumulator as textures here
}

void CVulkanRenderTarget::phase_wallmarks()
{
    VulkanBackend.set_RT(rt_Color->pRT, 0);
    VulkanBackend.set_RT(nullptr, 1);
    VulkanBackend.set_RT(nullptr, 2);
    VulkanBackend.set_ZB(VulkanHW.GetDepthRTView());
}
