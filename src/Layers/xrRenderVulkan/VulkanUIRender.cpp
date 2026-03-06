#include "stdafx.h"
#include "VulkanUIRender.h"
#include "VulkanUIShader.h"

CVulkanUIRender VulkanUIRenderImpl;

CVulkanUIRender::CVulkanUIRender()
{
    m_primType = ptNone;
    m_pointType = pttNone;
    m_iMaxVerts = 0;
    m_vOffset = 0;
    TL_start_pv = nullptr;
    TL_pv = nullptr;
    LIT_start_pv = nullptr;
    LIT_pv = nullptr;
}

CVulkanUIRender::~CVulkanUIRender()
{
}

void CVulkanUIRender::CreateUIGeom()
{
    hGeom_TL.create(D3DFVF_TL, RCache.Vertex.Buffer(), RCache.Index.Buffer());
    hGeom_LIT.create(D3DFVF_LIT, RCache.Vertex.Buffer(), RCache.Index.Buffer());
}

void CVulkanUIRender::DestroyUIGeom()
{
    hGeom_TL.destroy();
    hGeom_LIT.destroy();
}

void CVulkanUIRender::SetShader(IUIShader& shader)
{
    CVulkanUIShader& sh = (CVulkanUIShader&)shader;
    RCache.set_Shader(sh.GetShader());
}

void CVulkanUIRender::SetAlphaRef(int aref)
{
    RCache.set_AlphaRef(aref);
}

void CVulkanUIRender::SetScissor(Irect* rect)
{
    RCache.set_Scissor(rect);
}

void CVulkanUIRender::GetActiveTextureResolution(Fvector2& res)
{
    res.set(1024, 1024); // TODO: Get actual resolution from current shader's texture
    CTexture* T = RCache.get_ActiveTexture(0);
    if (T)
    {
        res.set((float)T->get_Width(), (float)T->get_Height());
    }
}

void CVulkanUIRender::PushPoint(float x, float y, float z, u32 C, float u, float v)
{
    switch (m_pointType)
    {
    case pttTL:
        TL_pv->set(x, y, z, 1.0f, C, u, v);
        TL_pv++;
        break;
    case pttLIT:
        LIT_pv->set(x, y, z, C, u, v);
        LIT_pv++;
        break;
    default:
        NODEFAULT;
    }
}

void CVulkanUIRender::StartPrimitive(u32 iMaxVerts, ePrimitiveType primType, ePointType pointType)
{
    m_iMaxVerts = iMaxVerts;
    m_primType = primType;
    m_pointType = pointType;

    switch (m_pointType)
    {
    case pttTL:
        TL_start_pv = (FVF::TL*)RCache.Vertex.Lock(m_iMaxVerts, hGeom_TL->vb_stride, m_vOffset);
        TL_pv = TL_start_pv;
        break;
    case pttLIT:
        LIT_start_pv = (FVF::LIT*)RCache.Vertex.Lock(m_iMaxVerts, hGeom_LIT->vb_stride, m_vOffset);
        LIT_pv = LIT_start_pv;
        break;
    default:
        NODEFAULT;
    }
}

void CVulkanUIRender::FlushPrimitive()
{
    u32 pCount = 0;
    u32 vCount = 0;

    switch (m_pointType)
    {
    case pttTL:
        vCount = (u32)(TL_pv - TL_start_pv);
        RCache.Vertex.Unlock(vCount, hGeom_TL->vb_stride);
        break;
    case pttLIT:
        vCount = (u32)(LIT_pv - LIT_start_pv);
        RCache.Vertex.Unlock(vCount, hGeom_LIT->vb_stride);
        break;
    default:
        NODEFAULT;
    }

    if (vCount == 0) return;

    switch (m_primType)
    {
    case ptTriList: pCount = vCount / 3; break;
    case ptTriStrip: pCount = vCount - 2; break;
    case ptLineStrip: pCount = vCount - 1; break;
    case ptLineList: pCount = vCount / 2; break;
    default: NODEFAULT;
    }

    D3DPRIMITIVETYPE pt;
    switch (m_primType)
    {
    case ptTriList: pt = D3DPT_TRIANGLELIST; break;
    case ptTriStrip: pt = D3DPT_TRIANGLESTRIP; break;
    case ptLineStrip: pt = D3DPT_LINESTRIP; break;
    case ptLineList: pt = D3DPT_LINELIST; break;
    default: NODEFAULT;
    }

    switch (m_pointType)
    {
    case pttTL:
        RCache.set_Geometry(hGeom_TL);
        break;
    case pttLIT:
        RCache.set_Geometry(hGeom_LIT);
        break;
    default:
        NODEFAULT;
    }

    RCache.Render(pt, m_vOffset, pCount);
}

LPCSTR CVulkanUIRender::UpdateShaderName(LPCSTR tex_name, LPCSTR sh_name)
{
    return sh_name;
}

void CVulkanUIRender::CacheSetXformWorld(const Fmatrix& M)
{
    RCache.set_xform_world(M);
}

void CVulkanUIRender::CacheSetCullMode(CullMode mode)
{
    switch (mode)
    {
    case cmNONE: RCache.set_CullMode(CULL_NONE); break;
    case cmCW: RCache.set_CullMode(CULL_CW); break;
    case cmCCW: RCache.set_CullMode(CULL_CCW); break;
    }
}
