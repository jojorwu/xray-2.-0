#pragma once

#include "../../Include/xrRender/RenderDeviceRender.h"

class CVulkanRenderDeviceRender : public IRenderDeviceRender
{
public:
    CVulkanRenderDeviceRender();
    virtual ~CVulkanRenderDeviceRender();

    virtual void Copy(IRenderDeviceRender& _in) override;

    // Gamma correction functions
    virtual void setGamma(float fGamma) override;
    virtual void setBrightness(float fGamma) override;
    virtual void setContrast(float fGamma) override;
    virtual void updateGamma() override;

    // Destroy
    virtual void OnDeviceDestroy(BOOL bKeepTextures) override;
    virtual void ValidateHW() override;
    virtual void DestroyHW() override;
    virtual void Reset(HWND hWnd, u32& dwWidth, u32& dwHeight, float& fWidth_2, float& fHeight_2) override;

    // Init
    virtual void SetupStates() override;
    virtual void OnDeviceCreate(LPCSTR shName) override;
    virtual void Create(HWND hWnd, u32& dwWidth, u32& dwHeight, float& fWidth_2, float& fHeight_2, bool) override;
    virtual void SetupGPU(BOOL bForceGPU_SW, BOOL bForceGPU_NonPure, BOOL bForceGPU_REF) override;

    // Overdraw
    virtual void overdrawBegin() override;
    virtual void overdrawEnd() override;

    // Resources control
    virtual void DeferredLoad(BOOL E) override;
    virtual void ResourcesDeferredUpload() override;
    virtual void ResourcesDeferredUnload() override;
    virtual void ResourcesGetMemoryUsage(u32& m_base, u32& c_base, u32& m_lmaps, u32& c_lmaps) override;
    virtual void ResourcesDestroyNecessaryTextures() override;
    virtual void ResourcesStoreNecessaryTextures() override;
    virtual void ResourcesDumpMemoryUsage() override;
    virtual void ResourcesPrefetchCreateTexture(LPCSTR name) override;

    // HWSupport
    virtual bool HWSupportsShaderYUV2RGB() override;

    // Device state
    virtual DeviceState GetDeviceState() override;
    virtual BOOL GetForceGPU_REF() override;
    virtual u32 GetCacheStatPolys() override;
    virtual void Begin() override;
    virtual void Clear() override;
    virtual void End() override;
    virtual void ClearTarget() override;
    virtual void SetCacheXform(Fmatrix& mView, Fmatrix& mProject) override;
    virtual void SetCacheXform_prev(Fmatrix& mView, Fmatrix& mProject) override;
    virtual void OnAssetsChanged() override;
};
