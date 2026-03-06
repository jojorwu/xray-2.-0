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
}

CVulkanRenderTarget::~CVulkanRenderTarget()
{
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
