#ifndef VulkanDebugRender_included
#define VulkanDebugRender_included
#pragma once

#include "../../Include/xrRender/DebugRender.h"

class CVulkanDebugRender : public IDebugRender
{
public:
    CVulkanDebugRender();
    virtual ~CVulkanDebugRender();

    virtual void Render() override;
    virtual void add_lines(Fvector const* vertices, u32 const& vertex_count, u16 const* pairs, u32 const& pair_count, u32 const& color, bool bHud = false) override;

    virtual void NextSceneMode() override;
    virtual void ZEnable(bool bEnable) override;
    virtual void OnFrameEnd() override;
    virtual void SetShader(const debug_shader& shader) override;
    virtual void CacheSetXformWorld(const Fmatrix& M) override;
    virtual void CacheSetCullMode(CullMode mode) override;
    virtual void SetAmbient(u32 colour) override;

    virtual void SetDebugShader(dbgShaderHandle shdHandle) override;
    virtual void DestroyDebugShader(dbgShaderHandle shdHandle) override;
    virtual void dbg_DrawTRI(Fmatrix& T, Fvector& p1, Fvector& p2, Fvector& p3, u32 C) override;

private:
    xr_unordered_map<u32, std::vector<FVF::L>> m_line_vertices;
    xr_unordered_map<u32, std::vector<u16>> m_line_indices;
    xr_unordered_map<u32, std::vector<FVF::L>> m_line_vertices_hud;
    xr_unordered_map<u32, std::vector<u16>> m_line_indices_hud;

    ref_shader m_dbgShaders[dbgShaderCount];
};

extern CVulkanDebugRender VulkanDebugRenderImpl;

#endif // VulkanDebugRender_included
