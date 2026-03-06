#include "stdafx.h"
#include "VulkanDebugRender.h"
#include "VulkanUIShader.h"

CVulkanDebugRender VulkanDebugRenderImpl;

CVulkanDebugRender::CVulkanDebugRender()
{
}

CVulkanDebugRender::~CVulkanDebugRender()
{
}

void CVulkanDebugRender::Render()
{
    RCache.set_xform_world(Fidentity);

    if (!m_line_vertices_hud.empty())
    {
        // Change projection
        Fmatrix FTold = Device.mFullTransform;
        Device.mFullTransform = Device.mFullTransformHud;
        RCache.set_xform_project(Device.mProjectHud);

        // Rendering
        ::Render->rmNear();

        for (auto& m_vert : m_line_vertices_hud)
        {
            const u32& color = m_vert.first;
            std::vector<FVF::L>& vert_vec = m_vert.second;
            auto& ind_vec = m_line_indices_hud.at(color);

            RCache.dbg_Draw(D3DPT_LINELIST, &vert_vec.front(), vert_vec.size(), &ind_vec.front(), ind_vec.size() / 2);
        }

        m_line_vertices_hud.clear();
        m_line_indices_hud.clear();

        ::Render->rmNormal();

        // Restore projection
        Device.mFullTransform = FTold;
        RCache.set_xform_project(Device.mProject);
    }

    if (m_line_vertices.empty())
        return;

    for (auto& m_vert : m_line_vertices)
    {
        const u32& color = m_vert.first;
        std::vector<FVF::L>& vert_vec = m_vert.second;
        auto& ind_vec = m_line_indices.at(color);

        RCache.dbg_Draw(D3DPT_LINELIST, &vert_vec.front(), vert_vec.size(), &ind_vec.front(), ind_vec.size() / 2);
    }

    m_line_vertices.clear();
    m_line_indices.clear();
}

void CVulkanDebugRender::add_lines(Fvector const* vertices, u32 const& vertex_count, u16 const* pairs, u32 const& pair_count, u32 const& color, bool bHud)
{
    size_t all_verts_count = 0, all_inds_count = 0;
    for (auto& m_vert : m_line_vertices)
    {
        const u32& color = m_vert.first;
        const std::vector<FVF::L>& vert_vec = m_vert.second;
        all_verts_count += vert_vec.size();
        all_inds_count += m_line_indices.at(color).size();
    }

    for (auto& m_vert : m_line_vertices_hud)
    {
        const u32& color = m_vert.first;
        const std::vector<FVF::L>& vert_vec = m_vert.second;
        all_verts_count += vert_vec.size();
        all_inds_count += m_line_indices_hud.at(color).size();
    }

    if (((all_verts_count + vertex_count) >= u16(-1)) || ((all_inds_count + 2 * pair_count) >= u16(-1)))
        Render();

    auto& vert_vec = bHud ? m_line_vertices_hud[color] : m_line_vertices[color];
    auto& ind_vec = bHud ? m_line_indices_hud[color] : m_line_indices[color];

    const auto vertices_size = vert_vec.size();
    const auto indices_size = ind_vec.size();

    ind_vec.resize(indices_size + 2 * pair_count);
    auto I = ind_vec.begin() + indices_size;
    const u16* J = pairs;
    for (u32 it = 0; it < 2 * pair_count; ++it, ++I, ++J)
        *I = (u16)(vertices_size + *J);

    vert_vec.resize(vertices_size + vertex_count);
    auto i = vert_vec.begin() + vertices_size;
    Fvector const* j = vertices;
    for (u32 it = 0; it < vertex_count; ++it, ++i, ++j) {
        i->color = color;
        i->p = *j;
    }
}

void CVulkanDebugRender::NextSceneMode()
{
}

void CVulkanDebugRender::ZEnable(bool bEnable)
{
    RCache.set_Z(bEnable);
}

void CVulkanDebugRender::OnFrameEnd()
{
}

void CVulkanDebugRender::SetShader(const debug_shader& shader)
{
    RCache.set_Shader(((CVulkanUIShader*)&*shader)->GetShader());
}

void CVulkanDebugRender::CacheSetXformWorld(const Fmatrix& M)
{
    RCache.set_xform_world(M);
}

void CVulkanDebugRender::CacheSetCullMode(CullMode m)
{
    RCache.set_CullMode(CULL_NONE + (u32)m);
}

void CVulkanDebugRender::SetAmbient(u32 colour)
{
}

void CVulkanDebugRender::SetDebugShader(dbgShaderHandle shdHandle)
{
    R_ASSERT(shdHandle < dbgShaderCount);

    static const LPCSTR dbgShaderParams[][2] =
    {
        {"hud\\default", "ui\\ui_pop_up_active_back"},
    };

    if (!m_dbgShaders[shdHandle])
        m_dbgShaders[shdHandle].create(
            dbgShaderParams[shdHandle][0], dbgShaderParams[shdHandle][1]);

    RCache.set_Shader(m_dbgShaders[shdHandle]);
}

void CVulkanDebugRender::DestroyDebugShader(dbgShaderHandle shdHandle)
{
    R_ASSERT(shdHandle < dbgShaderCount);
    m_dbgShaders[shdHandle].destroy();
}

void CVulkanDebugRender::dbg_DrawTRI(Fmatrix& T, Fvector& p1, Fvector& p2, Fvector& p3, u32 C)
{
    RCache.dbg_DrawTRI(T, p1, p2, p3, C);
}
