#pragma once

#include "../../xrcdb/ispatial.h"
#include "../../xrcdb/xr_collide_defs.h"

class CVulkanGlow : public IRender_Glow, public ISpatial
{
public:
    struct
    {
        u32 bActive : 1;
    } flags;

    float fade;
    ref_shader shader;
    u32 dwFrame;

    Fvector position;
    Fvector direction;
    float radius;
    Fcolor color;

    // Ray-testing cache
    BOOL bTestResult;
    collide::ray_cache RayCache;
    u32 qid_pass;
    u32 qid_total;

public:
    CVulkanGlow();
    virtual ~CVulkanGlow();

    virtual void set_active(bool b) override;
    virtual bool get_active() override;
    virtual void set_position(const Fvector& P) override;
    virtual void set_direction(const Fvector& D) override;
    virtual void set_radius(float R) override;
    virtual void set_texture(LPCSTR name) override;
    virtual void set_color(const Fcolor& C) override;
    virtual void set_color(float r, float g, float b) override;
    virtual void spatial_move() override;
};

class CGlowManager
{
    xr_vector<ref_glow> Glows;
    xr_vector<ref_glow> Selected;
public:
    void add(ref_glow g);

    void Load(IReader* fs);
    void Unload();

    void Render();

    CGlowManager();
    ~CGlowManager();
};
