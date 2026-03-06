#ifndef VulkanUIRender_included
#define VulkanUIRender_included
#pragma once

#include "../../Include/xrRender/UIRender.h"

class CVulkanUIRender : public IUIRender
{
public:
    CVulkanUIRender();
    virtual ~CVulkanUIRender();

    virtual void CreateUIGeom() override;
    virtual void DestroyUIGeom() override;

    virtual void SetShader(IUIShader& shader) override;
    virtual void SetAlphaRef(int aref) override;
    virtual void SetScissor(Irect* rect = nullptr) override;
    virtual void GetActiveTextureResolution(Fvector2& res) override;

    virtual void PushPoint(float x, float y, float z, u32 C, float u, float v) override;

    virtual void StartPrimitive(u32 iMaxVerts, ePrimitiveType primType, ePointType pointType) override;
    virtual void FlushPrimitive() override;

    virtual LPCSTR UpdateShaderName(LPCSTR tex_name, LPCSTR sh_name) override;

    virtual void CacheSetXformWorld(const Fmatrix& M) override;
    virtual void CacheSetCullMode(CullMode mode) override;

private:
    ref_geom hGeom_TL;
    ref_geom hGeom_LIT;

    ePrimitiveType m_primType;
    ePointType m_pointType;

    u32 m_iMaxVerts;
    u32 m_vOffset;

    FVF::TL* TL_start_pv;
    FVF::TL* TL_pv;

    FVF::LIT* LIT_start_pv;
    FVF::LIT* LIT_pv;
};

extern CVulkanUIRender VulkanUIRenderImpl;

#endif // VulkanUIRender_included
