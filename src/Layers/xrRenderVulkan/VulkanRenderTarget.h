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
    void phase_scene_begin();
    void phase_scene_end();
    void phase_accumulator();
    void phase_combine();
    void phase_wallmarks();
    void phase_bloom();
    void phase_dof();
    void phase_sunshafts();

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

private:
    u32 dwWidth;
    u32 dwHeight;
};
