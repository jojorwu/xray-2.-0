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

    accum_point_geom_create();
    accum_omnip_geom_create();
    accum_spot_geom_create();
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

void CVulkanRenderTarget::phase_depth_prepass()
{
    VulkanBackend.set_RT(nullptr, 0);
    VulkanBackend.set_RT(nullptr, 1);
    VulkanBackend.set_RT(nullptr, 2);
    VulkanBackend.set_ZB(VulkanHW.GetDepthRTView());
    VulkanBackend.Clear(); // Only depth since no RTs
}

void CVulkanRenderTarget::u_stencil_optimize(BOOL common_stencil)
{
    // Implementation for Vulkan stencil optimization
    VkClearRect rect = {};
    rect.rect.offset = { 0, 0 };
    rect.rect.extent = VulkanHW.GetSwapchainExtent();
    rect.baseArrayLayer = 0;
    rect.layerCount = 1;

    VkClearAttachment clear = {};
    clear.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
    clear.clearValue.depthStencil.stencil = 0;

    vkCmdClearAttachments(VulkanBackend.GetCurrentCommandBuffer(), 1, &clear, 1, &rect);

    // In a full implementation, we'd render the geometry of light volumes here
    // to mark the stencil buffer where lights are active.
}

#include "../xrRender/du_sphere.h"
#include "../xrRender/du_cone.h"
#include "VulkanResourceManager.h"

void CVulkanRenderTarget::accum_point_geom_create()
{
    CVulkanBuffer* vb = xr_new<CVulkanVertexBuffer>();
    vb->Create(DU_SPHERE_NUMVERTEX * sizeof(Fvector), false);
    void* mapped_vb;
    vb->Map(&mapped_vb);
    memcpy(mapped_vb, du_sphere_vertices, DU_SPHERE_NUMVERTEX * sizeof(Fvector));
    vb->Unmap();

    CVulkanBuffer* ib = xr_new<CVulkanIndexBuffer>();
    ib->Create(DU_SPHERE_NUMFACES * 3 * sizeof(WORD), false);
    void* mapped_ib;
    ib->Map(&mapped_ib);
    memcpy(mapped_ib, du_sphere_faces, DU_SPHERE_NUMFACES * 3 * sizeof(u16));
    ib->Unmap();

    g_accum_point.create(D3DFVF_XYZ, vb, ib);
}

void CVulkanRenderTarget::accum_omnip_geom_create()
{
    // Reuse sphere for omniparts too or create dedicated
    g_accum_omnipart = g_accum_point;
}

void CVulkanRenderTarget::accum_spot_geom_create()
{
    CVulkanBuffer* vb = xr_new<CVulkanVertexBuffer>();
    vb->Create(DU_CONE_NUMVERTEX * sizeof(Fvector), false);
    void* mapped_vb;
    vb->Map(&mapped_vb);
    memcpy(mapped_vb, du_cone_vertices, DU_CONE_NUMVERTEX * sizeof(Fvector));
    vb->Unmap();

    CVulkanBuffer* ib = xr_new<CVulkanIndexBuffer>();
    ib->Create(DU_CONE_NUMFACES * 3 * sizeof(WORD), false);
    void* mapped_ib;
    ib->Map(&mapped_ib);
    memcpy(mapped_ib, du_cone_faces, DU_CONE_NUMFACES * 3 * sizeof(u16));
    ib->Unmap();

    g_accum_spot.create(D3DFVF_XYZ, vb, ib);
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
    // Transition G-buffer for sampling
    VulkanBackend.TransitionRT(rt_Position->pRT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    VulkanBackend.TransitionRT(rt_Normal->pRT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    VulkanBackend.TransitionRT(rt_Color->pRT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    VulkanBackend.TransitionRT(rt_Accumulator->pRT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    VulkanBackend.set_RT(VulkanHW.GetSwapchainRTView(VulkanBackend.GetCurrentImageIndex()), 0);
    VulkanBackend.set_RT(nullptr, 1);
    VulkanBackend.set_RT(nullptr, 2);
    VulkanBackend.set_ZB(VulkanHW.GetDepthRTView());

    // Bind G-buffer textures
    VulkanBackend.SetTexture(0, rt_Position->pRT->view, VulkanHW.GetSampler());
    VulkanBackend.SetTexture(1, rt_Normal->pRT->view, VulkanHW.GetSampler());
    VulkanBackend.SetTexture(2, rt_Color->pRT->view, VulkanHW.GetSampler());
    VulkanBackend.SetTexture(3, rt_Accumulator->pRT->view, VulkanHW.GetSampler());
}

void CVulkanRenderTarget::phase_wallmarks()
{
    VulkanBackend.set_RT(rt_Color->pRT, 0);
    VulkanBackend.set_RT(nullptr, 1);
    VulkanBackend.set_RT(nullptr, 2);
    VulkanBackend.set_ZB(VulkanHW.GetDepthRTView());
}
