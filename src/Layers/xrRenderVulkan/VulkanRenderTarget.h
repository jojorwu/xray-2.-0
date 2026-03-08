#pragma once

#include "../xrRender/SH_RT.h"

class CVulkanRenderTarget : public IRender_Target
{
public:
    CVulkanRenderTarget();
    virtual ~CVulkanRenderTarget();

    virtual u32 get_width() override { return dwWidth; }
    virtual u32 get_height() override { return dwHeight; }

    void phase_scene_prepare();
    void phase_depth_prepass();
    void phase_scene_begin();
    void phase_scene_end();
    void phase_accumulator();
    void phase_combine();
    void phase_wallmarks();
    void accum_point_geom_create();
    void accum_omnip_geom_create();
    void accum_spot_geom_create();
    void phase_bloom();
    void phase_dof();
    void phase_sunshafts();
    void phase_smap_direct();
    void phase_smap_spot();
    void phase_smap_end();
    void u_stencil_optimize(BOOL common_stencil = TRUE);
    void render_screen_quad();

    // G-Buffer targets
    ref_rt rt_Position; // fat (x,y,z,?)
    ref_rt rt_Normal;   // fat (x,y,z,hemi)
    ref_rt rt_Color;    // fat (r,g,b,specular-gloss)

    // Accumulator target
    ref_rt rt_Accumulator; // (r,g,b,specular)

    // Post-process targets
    ref_rt rt_Bloom_1;
    ref_rt rt_Bloom_2;
    ref_rt rt_DOF;
    ref_rt rt_Sunshafts_0;
    ref_rt rt_Sunshafts_1;
    ref_rt rt_Generic_0;
    ref_rt rt_Generic_1;

    // Light volumes
    ref_geom g_accum_point;
    ref_geom g_accum_spot;
    ref_geom g_accum_omnipart;
    ref_geom g_screen_quad;

    // Shadow mapping
    ref_rt rt_smap_depth;
    ref_rt rt_smap_surf;

private:
    u32 dwWidth;
    u32 dwHeight;
};
