#include "stdafx.h"
#include "VulkanGlowManager.h"

CVulkanGlow::CVulkanGlow() : ISpatial(g_SpatialSpace)
{
    flags.bActive = false;
    position.set(0, 0, 0);
    direction.set(0, 0, 1);
    radius = 0.1f;
    color.set(1, 1, 1, 1);
    fade = 1.0f;
    dwFrame = 0;
    bTestResult = TRUE;
}

CVulkanGlow::~CVulkanGlow()
{
    set_active(false);
}

void CVulkanGlow::set_active(bool b)
{
    if (b)
    {
        if (flags.bActive) return;
        flags.bActive = true;
        spatial_register();
        spatial_move();
    }
    else
    {
        if (!flags.bActive) return;
        flags.bActive = false;
        spatial_move();
        spatial_unregister();
    }
}

bool CVulkanGlow::get_active()
{
    return flags.bActive;
}

void CVulkanGlow::set_position(const Fvector& P)
{
    position.set(P);
    spatial_move();
}

void CVulkanGlow::set_direction(const Fvector& D)
{
    direction.set(D);
    spatial_move();
}

void CVulkanGlow::set_radius(float R)
{
    radius = R;
    spatial_move();
}

void CVulkanGlow::set_texture(LPCSTR name)
{
    shader.create("effects\\glow", name);
}

void CVulkanGlow::set_color(const Fcolor& C)
{
    color.set(C);
}

void CVulkanGlow::set_color(float r, float g, float b)
{
    color.set(r, g, b, 1.0f);
}

void CVulkanGlow::spatial_move()
{
    spatial.sphere.set(position, radius);
    ISpatial::spatial_move();
}

CGlowManager::CGlowManager()
{
}

CGlowManager::~CGlowManager()
{
}

void CGlowManager::add(ref_glow g)
{
    Glows.push_back(g);
}

void CGlowManager::Load(IReader* fs)
{
}

void CGlowManager::Unload()
{
    Glows.clear();
}

void CGlowManager::Render()
{
    // TODO: Implement glow rendering
}
