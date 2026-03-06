#include "stdafx.h"
#include "VulkanRenderDeviceRender.h"
#include "VulkanResourceManager.h"

CVulkanRenderDeviceRender::CVulkanRenderDeviceRender()
{
    Resources = nullptr;
}

CVulkanRenderDeviceRender::~CVulkanRenderDeviceRender()
{
}

void CVulkanRenderDeviceRender::Copy(IRenderDeviceRender& _in)
{
}

void CVulkanRenderDeviceRender::setGamma(float fGamma)
{
}

void CVulkanRenderDeviceRender::setBrightness(float fGamma)
{
}

void CVulkanRenderDeviceRender::setContrast(float fGamma)
{
}

void CVulkanRenderDeviceRender::updateGamma()
{
}

void CVulkanRenderDeviceRender::OnDeviceDestroy(BOOL bKeepTextures)
{
}

void CVulkanRenderDeviceRender::ValidateHW()
{
}

void CVulkanRenderDeviceRender::DestroyHW()
{
    xr_delete(Resources);
}

void CVulkanRenderDeviceRender::Reset(HWND hWnd, u32& dwWidth, u32& dwHeight, float& fWidth_2, float& fHeight_2)
{
}

void CVulkanRenderDeviceRender::SetupStates()
{
}

void CVulkanRenderDeviceRender::OnDeviceCreate(LPCSTR shName)
{
}

void CVulkanRenderDeviceRender::Create(HWND hWnd, u32& dwWidth, u32& dwHeight, float& fWidth_2, float& fHeight_2, bool)
{
    VulkanRenderImpl.create();
    Resources = xr_new<CVulkanResourceManager>();
}

void CVulkanRenderDeviceRender::SetupGPU(BOOL bForceGPU_SW, BOOL bForceGPU_NonPure, BOOL bForceGPU_REF)
{
}

void CVulkanRenderDeviceRender::overdrawBegin()
{
}

void CVulkanRenderDeviceRender::overdrawEnd()
{
}

void CVulkanRenderDeviceRender::DeferredLoad(BOOL E)
{
}

void CVulkanRenderDeviceRender::ResourcesDeferredUpload()
{
}

void CVulkanRenderDeviceRender::ResourcesDeferredUnload()
{
}

void CVulkanRenderDeviceRender::ResourcesGetMemoryUsage(u32& m_base, u32& c_base, u32& m_lmaps, u32& c_lmaps)
{
}

void CVulkanRenderDeviceRender::ResourcesDestroyNecessaryTextures()
{
}

void CVulkanRenderDeviceRender::ResourcesStoreNecessaryTextures()
{
}

void CVulkanRenderDeviceRender::ResourcesDumpMemoryUsage()
{
}

void CVulkanRenderDeviceRender::ResourcesPrefetchCreateTexture(LPCSTR name)
{
}

bool CVulkanRenderDeviceRender::HWSupportsShaderYUV2RGB()
{
    return false;
}

CVulkanRenderDeviceRender::DeviceState CVulkanRenderDeviceRender::GetDeviceState()
{
    return dsOK;
}

BOOL CVulkanRenderDeviceRender::GetForceGPU_REF()
{
    return FALSE;
}

u32 CVulkanRenderDeviceRender::GetCacheStatPolys()
{
    return 0;
}

void CVulkanRenderDeviceRender::Begin()
{
    if (!VulkanRenderImpl.Begin())
    {
        // Handle failure to begin frame
    }
}

void CVulkanRenderDeviceRender::Clear()
{
    VulkanBackend.Clear();
}

void CVulkanRenderDeviceRender::End()
{
    VulkanRenderImpl.End();
}

void CVulkanRenderDeviceRender::ClearTarget()
{
    VulkanBackend.ClearTarget();
}

void CVulkanRenderDeviceRender::SetCacheXform(Fmatrix& mView, Fmatrix& mProject)
{
}

void CVulkanRenderDeviceRender::SetCacheXform_prev(Fmatrix& mView, Fmatrix& mProject)
{
}

void CVulkanRenderDeviceRender::OnAssetsChanged()
{
}
